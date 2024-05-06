// SPDX-License-Identifier: MIT

#include <atomic>
#include <chrono>
#include <cmath>                // fabs()
#include <cstdio>
#include <numeric>              // accumulate()
#include <ranges>               // views::zip()
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// WUT/WUPS and unix headers
#include <coreinit/time.h>
#include <nn/ccr.h>             // CCRSysSetSystemTime()
#include <nn/pdm.h>             // __OSSetAbsoluteSystemTime()
#include <sys/select.h>         // select()
#include <sys/socket.h>         // connect(), send(), recv()

#include "cfg.hpp"
#include "core.hpp"
#include "limited_async.hpp"
#include "log.hpp"
#include "ntp.hpp"
#include "utc.hpp"
#include "utils.hpp"


using namespace std::literals;


std::counting_semaphore<> async_limit{5}; // limit to 5 threads


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
    ntp_query(struct sockaddr_in address)
    {
        using std::to_string;

        utils::socket_guard s{PF_INET, SOCK_DGRAM, IPPROTO_UDP};

        connect(s.fd, reinterpret_cast<struct sockaddr*>(&address), sizeof address);

        ntp::packet packet;
        packet.version(4);
        packet.mode(ntp::packet::mode_flag::client);


        unsigned num_send_tries = 0;
    try_again_send:
        auto t1 = to_ntp(utc::now());
        packet.transmit_time = t1;

        if (send(s.fd, &packet, sizeof packet, 0) == -1) {
            int e = errno;
            if (e != ENOMEM)
                throw std::runtime_error{"send() failed: "s
                                         + utils::errno_to_string(e)};
            if (++num_send_tries < 4) {
                std::this_thread::sleep_for(100ms);
                goto try_again_send;
            } else
                throw std::runtime_error{"No resources for send(), too many retries!"};
        }


        struct timeval timeout = { 4, 0 };
        fd_set read_set;
        unsigned num_select_tries = 0;
    try_again_select:
        FD_ZERO(&read_set);
        FD_SET(s.fd, &read_set);

        if (select(s.fd + 1, &read_set, nullptr, nullptr, &timeout) == -1) {
            // Wii U's OS can only handle 16 concurrent select() calls,
            // so we may need to try again later.
            int e = errno;
            if (e != ENOMEM)
                throw std::runtime_error{"select() failed: "s
                                         + utils::errno_to_string(e)};
            if (++num_select_tries < 4) {
                std::this_thread::sleep_for(10ms);
                goto try_again_select;
            } else
                throw std::runtime_error{"No resources for select(), too many retries!"};
        }

        if (!FD_ISSET(s.fd, &read_set))
            throw std::runtime_error{"Timeout reached!"};

        // Measure the arrival time as soon as possible.
        auto t4 = to_ntp(utc::now());

        if (recv(s.fd, &packet, sizeof packet, 0) < 48)
            throw std::runtime_error{"Invalid NTP response!"};

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
    apply_clock_correction(double correction)
    {
        OSTime correction_ticks = correction * OSTimerClockSpeed;

        nn::pdm::NotifySetTimeBeginEvent();

        OSTime now = OSGetTime();
        OSTime corrected = now + correction_ticks;

        if (CCRSysSetSystemTime(corrected)) {
            nn::pdm::NotifySetTimeEndEvent();
            return false;
        }

        bool res = __OSSetAbsoluteSystemTime(corrected);

        nn::pdm::NotifySetTimeEndEvent();

        return res;
    }



    void
    sync_clock()
    {
        using utils::seconds_to_human;

        if (!cfg::sync)
            return;

        static std::atomic<bool> executing = false;

        utils::exec_guard guard{executing};
        if (!guard.guarded) {
            // Another thread is already executing this function.
            report_info("Skipping NTP task: already in progress.");
            return;
        }

        cfg::update_utc_offset();

        std::vector<std::string> servers = utils::split(cfg::server, " \t,;");

        utils::addrinfo_query query = {
            .family = AF_INET,
            .socktype = SOCK_DGRAM,
            .protocol = IPPROTO_UDP
        };

        // First, resolve all the names, in parallel.
        // Some IP addresses might be duplicated when we use *.pool.ntp.org.
        std::set<struct sockaddr_in,
                 utils::less_sockaddr_in> addresses;
        {
            using info_vec = std::vector<utils::addrinfo_result>;
            std::vector<std::future<info_vec>> futures(servers.size());

            // Launch DNS queries asynchronously.
            for (auto [fut, server] : std::views::zip(futures, servers))
                fut = limited_async(utils::get_address_info, server, "123", query);

            // Collect all future results.
            for (auto& fut : futures)
                try {
                    for (auto info : fut.get())
                        addresses.insert(info.address);
                }
                catch (std::exception& e) {
                    report_error(e.what());
                }
        }

        // Launch NTP queries asynchronously.
        std::vector<std::future<std::pair<double, double>>> futures(addresses.size());
        for (auto [fut, address] : std::views::zip(futures, addresses))
            fut = limited_async(ntp_query, address);

        // Collect all future results.
        std::vector<double> corrections;
        for (auto [address, fut] : std::views::zip(addresses, futures))
            try {
                auto [correction, latency] = fut.get();
                corrections.push_back(correction);
                report_info(utils::to_string(address)
                            + ": correction = "s + seconds_to_human(correction)
                            + ", latency = "s + seconds_to_human(latency));
            }
            catch (std::exception& e) {
                report_error(utils::to_string(address) + ": "s + e.what());
            }


        if (corrections.empty()) {
            report_error("No NTP server could be used!");
            return;
        }

        double avg_correction = std::accumulate(corrections.begin(),
                                                corrections.end(),
                                                0.0)
            / corrections.size();

        if (std::fabs(avg_correction) * 1000 <= cfg::tolerance) {
            report_success("Tolerating clock drift (correction is only "
                           + seconds_to_human(avg_correction) + ")."s);
            return;
        }

        if (cfg::sync) {
            if (!apply_clock_correction(avg_correction)) {
                report_error("Failed to set system clock!");
                return;
            }
            report_success("Clock corrected by " + seconds_to_human(avg_correction));
        }

    }


    std::string
    local_clock_to_string()
    {
        return ticks_to_string(OSGetTime());
    }

} // namespace core
