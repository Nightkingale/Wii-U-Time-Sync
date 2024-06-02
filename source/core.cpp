// SPDX-License-Identifier: MIT

#include <atomic>
#include <chrono>
#include <cmath>                // fabs()
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

#include "core.hpp"

#include "cfg.hpp"
#include "logging.hpp"
#include "net/addrinfo.hpp"
#include "net/socket.hpp"
#include "notify.hpp"
#include "ntp.hpp"
#include "thread_pool.hpp"
#include "utc.hpp"
#include "utils.hpp"


using namespace std::literals;


namespace {

    // Difference from NTP (1900) to Wii U (2000) epochs.
    // There are 24 leap years in this period.
    constexpr double seconds_per_day = 24 * 60 * 60;
    constexpr double epoch_diff = seconds_per_day * (100 * 365 + 24);


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
        return utc::timestamp{static_cast<double>(t) - epoch_diff};
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
        OSTime ticks = ut.value * OSTimerClockSpeed;
        return ticks_to_string(ticks);
    }

}


namespace core {

    // Note: hardcoded for IPv4, the Wii U doesn't have IPv6.
    std::pair<double, double>
    ntp_query(net::address address)
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
        auto t1 = to_ntp(utc::now());
        packet.transmit_time = t1;

        auto send_status = sock.try_send(&packet, sizeof packet);
        if (!send_status) {
            auto& e = send_status.error();
            if (e.code() != std::errc::not_enough_memory)
                throw e;
            if (++send_attempts < max_send_attempts) {
                std::this_thread::sleep_for(100ms);
                goto try_again_send;
            } else
                throw std::runtime_error{"No resources for send(), too many retries!"};
        }


        unsigned poll_attempts = 0;
        const unsigned max_poll_attempts = 4;
    try_again_poll:
        auto readable_status = sock.try_is_readable(4s);
        if (!readable_status) {
            // Wii U OS can only handle 16 concurrent select()/poll() calls,
            // so we may need to try again later.
            auto& e = readable_status.error();
            if (e.code() != std::errc::not_enough_memory)
                throw e;
            if (++poll_attempts < max_poll_attempts) {
                std::this_thread::sleep_for(10ms);
                goto try_again_poll;
            } else
                throw std::runtime_error{"No resources for poll(), too many retries!"};
        }

        if (!*readable_status)
            throw std::runtime_error{"Timeout reached!"};

        // Measure the arrival time as soon as possible.
        auto t4 = to_ntp(utc::now());

        if (sock.recv(&packet, sizeof packet) < 48)
            throw std::runtime_error{"Invalid NTP response!"};

        sock.close(); // close it early

        auto v = packet.version();
        if (v < 3 || v > 4)
            throw std::runtime_error{"Unsupported NTP version: "s + to_string(v)};

        auto m = packet.mode();
        if (m != ntp::packet::mode_flag::server)
            throw std::runtime_error{"Invalid NTP packet mode: "s + to_string(m)};

        auto l = packet.leap();
        if (l == ntp::packet::leap_flag::unknown)
            throw std::runtime_error{"Unknown value for leap flag."};

        ntp::timestamp t1_received = packet.origin_time;
        if (t1 != t1_received)
            throw std::runtime_error{"NTP response mismatch: ["s
                                     + ::to_string(t1) + "] vs ["s
                                     + ::to_string(t1_received) + "]"s};

        // when our request arrived at the server
        auto t2 = packet.receive_time;
        // when the server sent out a response
        auto t3 = packet.transmit_time;

        // Zero is not a valid timestamp.
        if (!t2 || !t3)
            throw std::runtime_error{"NTP response has invalid timestamps."};

        /*
         * We do all calculations in double precision to never worry about overflows. Since
         * double precision has 53 mantissa bits, we're guaranteed to have 53 - 32 = 21
         * fractional bits in Era 0, and 20 fractional bits in Era 1 (starting in 2036). We
         * still have sub-microsecond resolution.
         */
        double d1 = static_cast<double>(t1);
        double d2 = static_cast<double>(t2);
        double d3 = static_cast<double>(t3);
        double d4 = static_cast<double>(t4);

        // Detect the wraparound that will happen at the end of Era 0.
        if (d4 < d1)
            d4 += 0x1.0p32; // d4 += 2^32
        if (d3 < d2)
            d3 += 0x1.0p32; // d3 += 2^32

        double roundtrip = (d4 - d1) - (d3 - d2);
        double latency = roundtrip / 2;

        // t4 + correction = t3 + latency
        double correction = d3 + latency - d4;

        /*
         * If the local clock enters Era 1 ahead of NTP, we get a massive positive correction
         * because the local clock wrapped back to zero.
         */
        if (correction > 0x1.0p31) // if correcting more than 68 years forward
            correction -= 0x1.0p32;

        /*
         * If NTP enters Era 1 ahead of the local clock, we get a massive negative correction
         * because NTP wrapped back to zero.
         */
        if (correction < -0x1.0p31) // if correcting more than 68 years backward
            correction += 0x1.0p32;

        return { correction, latency };
    }



    bool
    apply_clock_correction(double seconds)
    {
        // OSTime before = OSGetSystemTime();

        OSTime ticks = seconds * OSTimerClockSpeed;

        nn::pdm::NotifySetTimeBeginEvent();

        // OSTime ccr_start = OSGetSystemTime();
        bool success1 = !CCRSysSetSystemTime(OSGetTime() + ticks);
        // OSTime ccr_finish = OSGetSystemTime();

        // OSTime abs_start = OSGetSystemTime();
        bool success2 = __OSSetAbsoluteSystemTime(OSGetTime() + ticks);
        // OSTime abs_finish = OSGetSystemTime();

        nn::pdm::NotifySetTimeEndEvent();

        // logging::printf("CCRSysSetSystemTime() took %f ms",
        //                 1000.0 * (ccr_finish - ccr_start) / OSTimerClockSpeed);
        // logging::printf("__OSSetAbsoluteSystemTime() took %f ms",
        //                 1000.0 * (abs_finish - abs_start) / OSTimerClockSpeed);

        // OSTime after = OSGetSystemTime();
        // logging::printf("Total time: %f ms",
        //                 1000.0 * (after - before) / OSTimerClockSpeed);

        return success1 && success2;
    }


    void
    run()
    {
        using utils::seconds_to_human;

        if (!cfg::sync)
            return;

        // ensure notification is initialized if needed
        notify::guard notify_guard{cfg::notify > 0};

        static std::atomic<bool> executing = false;
        utils::exec_guard exec_guard{executing};
        if (!exec_guard.guarded) {
            // Another thread is already executing this function.
            notify::info(notify::level::verbose,
                         "Skipping NTP task: operation already in progress.");
            return;
        }


        if (cfg::auto_tz) {
            try {
                auto [name, offset] = utils::fetch_timezone();
                if (offset != cfg::get_utc_offset()) {
                    cfg::set_utc_offset(offset);
                    notify::info(notify::level::verbose,
                                 "Auto-updated time zone to " + name +
                                 "(" + utils::tz_offset_to_string(offset) + ")");
                }
            }
            catch (std::exception& e) {
                notify::error(notify::level::verbose,
                              "Failed to auto-update time zone: "s + e.what());
            }
        }

        thread_pool pool(cfg::threads);


        std::vector<std::string> servers = utils::split(cfg::server, " \t,;");

        // First, resolve all the names, in parallel.
        // Some IP addresses might be duplicated when we use "pool.ntp.org".
        std::set<net::address> addresses;
        {
            // nested scope so the futures vector is destroyed
            using info_vec = std::vector<net::addrinfo::result>;
            std::vector<std::future<info_vec>> futures(servers.size());

            net::addrinfo::hints opts{ .type = net::socket::type::udp };
            // Launch DNS queries asynchronously.
            for (auto [fut, server] : std::views::zip(futures, servers))
                fut = pool.submit(net::addrinfo::lookup, server, "123"s, opts);

            // Collect all future results.
            for (auto& fut : futures)
                try {
                    for (auto info : fut.get())
                        addresses.insert(info.addr);
                }
                catch (std::exception& e) {
                    notify::error(notify::level::verbose, e.what());
                }
        }

        if (addresses.empty()) {
            // Probably a mistake in config, or network failure.
            notify::error(notify::level::normal, "No NTP address could be used.");
            return;
        }

        // Launch NTP queries asynchronously.
        std::vector<std::future<std::pair<double, double>>> futures(addresses.size());
        for (auto [fut, address] : std::views::zip(futures, addresses))
            fut = pool.submit(ntp_query, address);

        // Collect all future results.
        std::vector<double> corrections;
        for (auto [address, fut] : std::views::zip(addresses, futures))
            try {
                auto [correction, latency] = fut.get();
                corrections.push_back(correction);
                notify::info(notify::level::verbose,
                             to_string(address)
                             + ": correction = "s + seconds_to_human(correction, true)
                             + ", latency = "s + seconds_to_human(latency));
            }
            catch (std::exception& e) {
                notify::error(notify::level::verbose,
                              to_string(address) + ": "s + e.what());
            }


        if (corrections.empty()) {
            notify::error(notify::level::normal,
                          "No NTP server could be used!");
            return;
        }

        double avg_correction = std::accumulate(corrections.begin(),
                                                corrections.end(),
                                                0.0)
            / corrections.size();

        if (std::fabs(avg_correction) * 1000 <= cfg::tolerance) {
            notify::success(notify::level::verbose,
                            "Tolerating clock drift (correction is only "
                            + seconds_to_human(avg_correction, true) + ")."s);
            return;
        }

        if (cfg::sync) {
            if (!apply_clock_correction(avg_correction)) {
                // This error woudl be so bad, the user should always know about it.
                notify::error(notify::level::quiet,
                              "Failed to set system clock!");
                return;
            }
            notify::success(notify::level::normal,
                            "Clock corrected by " + seconds_to_human(avg_correction, true));
        }

    }


    std::string
    local_clock_to_string()
    {
        return ticks_to_string(OSGetTime());
    }

} // namespace core
