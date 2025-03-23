/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2025  Daniel K. O.
 * Copyright (C) 2024  Nightkingale
 *
 * SPDX-License-Identifier: MIT
 */

#include <atomic>
#include <chrono>
#include <cstdio>               // snprintf()
#include <numeric>              // accumulate()
#include <ranges>               // views::zip()
#include <set>
#include <stdexcept>            // runtime_error
#include <string>
#include <thread>
#include <vector>

#include <coreinit/time.h>
#include <nn/ccr.h>             // CCRSysSetSystemTime()
#include <nn/pdm.h>             // __OSSetAbsoluteSystemTime()

#include <wupsxx/logger.hpp>

#include "core.hpp"

#include "cfg.hpp"
#include "net/addrinfo.hpp"
#include "net/socket.hpp"
#include "notify.hpp"
#include "ntp.hpp"
#include "thread_pool.hpp"
#include "time_utils.hpp"
#include "utc.hpp"
#include "utils.hpp"


using namespace std::literals;
using std::runtime_error;

namespace logger = wups::logger;

using time_utils::dbl_seconds;


namespace {

    // Difference from NTP (1900) to Wii U (2000) epochs.
    // There are 24 leap years in this period.
    constexpr dbl_seconds seconds_per_day{24 * 60 * 60};
    constexpr dbl_seconds epoch_diff = seconds_per_day * (100 * 365 + 24);


    // Wii U -> NTP epoch.
    ntp::timestamp
    to_ntp(utc::timestamp t)
    {
        return ntp::timestamp{t.value + epoch_diff};
    }


    // NTP -> Wii U epoch.
    utc::timestamp
    to_utc(ntp::timestamp t)
    {
        return utc::timestamp{static_cast<dbl_seconds>(t) - epoch_diff};
    }


    std::string
    ticks_to_string(OSTime wt)
    {
        OSCalendarTime cal;
        OSTicksToCalendarTime(wt, &cal);
        char buffer[256];
        std::snprintf(buffer, sizeof buffer,
                      "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                      cal.tm_year, cal.tm_mon + 1, cal.tm_mday,
                      cal.tm_hour, cal.tm_min, cal.tm_sec, cal.tm_msec);
        return buffer;
    }


    std::string
    to_string(ntp::timestamp t)
    {
        auto ut = to_utc(t);
        OSTime ticks = ut.value.count() * OSTimerClockSpeed;
        return ticks_to_string(ticks);
    }

} // namespace


namespace core {


    struct canceled_error : runtime_error {
        canceled_error() : runtime_error{"Operation canceled."} {}
    };


    void
    check_stop(std::stop_token token)
    {
        if (token.stop_requested())
            throw canceled_error{};
    }


    void
    sleep_for(std::chrono::milliseconds t,
              std::stop_token token)
    {
        using clock = std::chrono::steady_clock;
        const auto deadline = clock::now() + t;
        while (clock::now() < deadline) {
            check_stop(token);
            std::this_thread::sleep_for(100ms);
        }
    }


    // Note: hardcoded for IPv4, the Wii U doesn't have IPv6.
    std::pair<dbl_seconds, dbl_seconds>
    ntp_query(std::stop_token token,
              net::address address)
    {
        using std::to_string;

        net::socket sock{net::socket::type::udp};
        sock.connect(address);

        ntp::packet packet;
        packet.version(4);
        packet.mode(ntp::packet::mode_flag::client);


        unsigned send_attempts = 0;
        const unsigned max_send_attempts = 4;

    try_again_send:
        // cancellation point: before sending
        check_stop(token);
        auto t1 = to_ntp(utc::now());
        packet.transmit_time = t1;

        auto send_status = sock.try_send(&packet, sizeof packet);
        if (!send_status) {
            auto& e = send_status.error();
            if (e.code() != std::errc::not_enough_memory)
                throw e;
            if (++send_attempts < max_send_attempts) {
                // cancellation point: before sleeping
                check_stop(token);
                std::this_thread::sleep_for(100ms);
                goto try_again_send;
            } else
                throw runtime_error{"No resources for send(), too many retries!"};
        }


        unsigned poll_attempts = 0;
        const unsigned max_poll_attempts = 4;

    try_again_poll:
        // cancellation point: before polling
        check_stop(token);
        auto readable_status = sock.try_is_readable(cfg::timeout.value);
        if (!readable_status) {
            // Wii U OS can only handle 16 concurrent select()/poll() calls,
            // so we may need to try again later.
            auto& e = readable_status.error();
            if (e.code() != std::errc::not_enough_memory)
                throw e;
            if (++poll_attempts < max_poll_attempts) {
                // cancellation point: before sleeping
                check_stop(token);
                std::this_thread::sleep_for(10ms);
                goto try_again_poll;
            } else
                throw runtime_error{"No resources for poll(), too many retries!"};
        }

        if (!*readable_status)
            throw runtime_error{"Timeout reached!"};

        // Measure the arrival time as soon as possible.
        auto t4 = to_ntp(utc::now());

        if (sock.recv(&packet, sizeof packet) < 48)
            throw runtime_error{"Invalid NTP response!"};

        sock.close(); // close it early

        auto v = packet.version();
        if (v < 3 || v > 4)
            throw runtime_error{"Unsupported NTP version: "s + to_string(v)};

        auto m = packet.mode();
        if (m != ntp::packet::mode_flag::server)
            throw runtime_error{"Invalid NTP packet mode: "s + to_string(m)};

        auto l = packet.leap();
        if (l == ntp::packet::leap_flag::unknown)
            throw runtime_error{"Unknown value for leap flag."};

        ntp::timestamp t1_received = packet.origin_time;
        if (t1 != t1_received)
            throw runtime_error{"NTP response mismatch: ["s
                                + ::to_string(t1) + "] vs ["s
                                + ::to_string(t1_received) + "]"s};

        // when our request arrived at the server
        auto t2 = packet.receive_time;
        // when the server sent out a response
        auto t3 = packet.transmit_time;

        // Zero is not a valid timestamp.
        if (!t2 || !t3)
            throw runtime_error{"NTP response has invalid timestamps."};

        /*
         * We do all calculations in double precision to never worry about overflows. Since
         * double precision has 53 mantissa bits, we're guaranteed to have 53 - 32 = 21
         * fractional bits in Era 0, and 20 fractional bits in Era 1 (starting in 2036). We
         * still have sub-microsecond resolution.
         */
        auto d1 = static_cast<dbl_seconds>(t1);
        auto d2 = static_cast<dbl_seconds>(t2);
        auto d3 = static_cast<dbl_seconds>(t3);
        auto d4 = static_cast<dbl_seconds>(t4);

        // Detect the wraparound that will happen at the end of Era 0.
        constexpr dbl_seconds half_era{0x1.0p32};     // 2^32 seconds
        constexpr dbl_seconds quarter_era{0x1.0p31};  // 2^31 seconds
        if (d4 < d1)
            d4 += half_era; // d4 += 2^32
        if (d3 < d2)
            d3 += half_era; // d3 += 2^32

        dbl_seconds roundtrip = (d4 - d1) - (d3 - d2);
        dbl_seconds latency = roundtrip / 2.0;

        // t4 + correction = t3 + latency
        dbl_seconds correction = d3 + latency - d4;

        /*
         * If the local clock enters Era 1 ahead of NTP, we get a massive positive correction
         * because the local clock wrapped back to zero.
         */
        if (correction > quarter_era) // if correcting more than 68 years forward
            correction -= half_era;

        /*
         * If NTP enters Era 1 ahead of the local clock, we get a massive negative correction
         * because NTP wrapped back to zero.
         */
        if (correction < -quarter_era) // if correcting more than 68 years backward
            correction += half_era;

        return { correction, latency };
    }



    bool
    apply_clock_correction(dbl_seconds seconds)
    {
        // OSTime before = OSGetSystemTime();

        OSTime ticks = seconds.count() * OSTimerClockSpeed;

        nn::pdm::NotifySetTimeBeginEvent();

        // OSTime ccr_start = OSGetSystemTime();
        bool success1 = !CCRSysSetSystemTime(OSGetTime() + ticks);
        // OSTime ccr_finish = OSGetSystemTime();

        // OSTime abs_start = OSGetSystemTime();
        bool success2 = __OSSetAbsoluteSystemTime(OSGetTime() + ticks);
        // OSTime abs_finish = OSGetSystemTime();

        nn::pdm::NotifySetTimeEndEvent();

        // logger::printf("CCRSysSetSystemTime() took %f ms\n",
        //                1000.0 * (ccr_finish - ccr_start) / OSTimerClockSpeed);
        // logger::printf("__OSSetAbsoluteSystemTime() took %f ms\n",
        //                1000.0 * (abs_finish - abs_start) / OSTimerClockSpeed);

        // OSTime after = OSGetSystemTime();
        // logger::printf("Total time: %f ms\n",
        //                1000.0 * (after - before) / OSTimerClockSpeed);

        return success1 && success2;
    }


    void
    run(std::stop_token token,
        bool silent)
    {
        using time_utils::seconds_to_human;

        utils::network_guard net_guard;

        static std::atomic<bool> executing = false;
        utils::exec_guard exec_guard{executing};
        if (!exec_guard.guarded) {
            // Another thread is already executing this function.
            throw runtime_error{"Skipping NTP task: operation already in progress."};
        }

        if (cfg::auto_tz.value) {
            try {
                auto [name, offset] = utils::fetch_timezone(cfg::tz_service.value);
                if (offset != cfg::utc_offset.value) {
                    cfg::set_and_store_utc_offset(offset);
                    if (!silent)
                        notify::info(notify::level::verbose,
                                     "Updated time zone to %s (%s)",
                                     name.data(),
                                     time_utils::tz_offset_to_string(offset).data());
                }
            }
            catch (std::exception& e) {
                if (!silent)
                    notify::error(notify::level::verbose,
                                  "Failed to update time zone: %s",
                                  e.what());
                // Note: not a fatal error, we just keep using the previous time zone.
            }
        }

        // cancellation point: after the time zone update
        check_stop(token);

        thread_pool pool{static_cast<unsigned>(cfg::threads.value)};

        std::vector<std::string> servers = utils::split(cfg::server.value, " \t,;");

        // First, resolve all the names, in parallel.
        // Some IP addresses might be duplicated when we use "pool.ntp.org".
        std::set<net::address> addresses;
        {
            // nested scope so the futures vector is destroyed early
            using info_vec = std::vector<net::addrinfo::result>;
            std::vector<std::future<info_vec>> futures(servers.size());

            net::addrinfo::hints opts{ .type = net::socket::type::udp };
            // Launch DNS queries asynchronously.
            for (auto [fut, server] : std::views::zip(futures, servers))
                fut = pool.submit(net::addrinfo::lookup, server, "123"s, opts);

            // cancellation point: after submitting the DNS queries
            check_stop(token);

            // Collect all future results.
            for (auto& fut : futures)
                try {
                    for (auto info : fut.get())
                        addresses.insert(info.addr);
                }
                catch (std::exception& e) {
                    if (!silent)
                        notify::error(notify::level::verbose, "%s", e.what());
                }
        }

        if (addresses.empty()) {
            // Probably a mistake in config, or network failure.
            throw runtime_error{"No NTP address could be used."};
        }

        // cancellation point: before the NTP queries are submitted
        check_stop(token);

        // Launch NTP queries asynchronously.
        std::vector<std::future<std::pair<dbl_seconds, dbl_seconds>>>
            futures(addresses.size());
        for (auto [fut, address] : std::views::zip(futures, addresses))
            fut = pool.submit(ntp_query, token, address);

        // cancellation point: after NTP queries are submited
        check_stop(token);

        // Collect all future results.
        std::vector<dbl_seconds> corrections;
        for (auto [address, fut] : std::views::zip(addresses, futures))
            try {
                // cancellation point: before blocking waiting for a NTP result
                check_stop(token);
                auto [correction, latency] = fut.get();
                corrections.push_back(correction);
                if (!silent)
                    notify::info(notify::level::verbose,
                                 "%s: correction = %s, latency = %s",
                                 to_string(address).data(),
                                 seconds_to_human(correction, true).data(),
                                 seconds_to_human(latency).data());
            }
            catch (canceled_error&) {
                throw;
            }
            catch (std::exception& e) {
                if (!silent)
                    notify::error(notify::level::verbose,
                                  "%s: %s",
                                  to_string(address).data(),
                                  e.what());
            }


        if (corrections.empty())
            throw runtime_error{"No NTP server could be used!"};


        dbl_seconds total = std::accumulate(corrections.begin(),
                                            corrections.end(),
                                            dbl_seconds{0});
        dbl_seconds avg = total / static_cast<double>(corrections.size());

        if (abs(avg) <= cfg::tolerance.value) {
            if (!silent)
                notify::success(notify::level::verbose,
                                "Tolerating clock drift (correction is only %s).",
                                seconds_to_human(avg, true).data());
            return;
        }

        // cancellation point: before modifying the clock
        check_stop(token);

        if (!apply_clock_correction(avg))
            throw runtime_error{"Failed to set system clock!"};

        if (!silent)
            notify::success(notify::level::normal,
                            "Clock corrected by %s",
                            seconds_to_human(avg, true).data());

    }


    std::string
    local_clock_to_string()
    {
        return ticks_to_string(OSGetTime());
    }


    namespace background {

        std::stop_source stopper{std::nostopstate};

        enum class state_t : unsigned {
            none,
            started,
            finished,
            canceled,
        };
        std::atomic<state_t> state{state_t::none};


        void
        run()
        {
            state = state_t::started;

            std::jthread t{
                [](std::stop_token token)
                {
                    wups::logger::guard logger_guard;

                    try {
                        // Note: we wait 5 seconds, to minimize spurious network errors.
                        sleep_for(5s, token);
                        core::run(token, false);
                        state = state_t::finished;
                    }
                    catch (canceled_error& e) {
                        state = state_t::canceled;
                    }
                    catch (std::exception& e) {
                        notify::error(notify::level::normal, "%s", e.what());
                        state = state_t::finished;
                    }
                }
            };

            stopper = t.get_stop_source();

            t.detach();
        }


        void
        run_once()
        {
            if (state != state_t::finished)
                run();
        }


        void
        stop()
        {
            if (state == state_t::started) {
                stopper.request_stop();

                // Wait up to ~10 seconds for the thread to flag it stopped running.
                unsigned max_tries = 100;
                do {
                    std::this_thread::sleep_for(100ms);
                } while ((state == state_t::started) && --max_tries);

                if (state == state_t::started)
                    logger::printf("WARNING: Background thread did not stop!\n");

                stopper = std::stop_source{std::nostopstate};
            }
        }

    } // namespace background

} // namespace core
