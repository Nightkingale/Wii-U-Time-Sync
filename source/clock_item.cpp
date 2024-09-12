/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 * Copyright (C) 2024  Nightkingale
 *
 * SPDX-License-Identifier: MIT
 */

#include <cmath>                // max(), min()
#include <exception>
#include <vector>

#include <wupsxx/cafe_glyphs.h>
#include <wupsxx/logger.hpp>

#include "clock_item.hpp"

#include "cfg.hpp"
#include "core.hpp"
#include "net/addrinfo.hpp"
#include "time_utils.hpp"
#include "utils.hpp"


using namespace std::literals;
using namespace wups::config;

using time_utils::dbl_seconds;

namespace logger = wups::logger;


namespace {

    template<typename T>
    struct statistics {
        T min = T{0};
        T max = T{0};
        T avg = T{0};
    };


    template<typename T>
    statistics<T>
    get_statistics(const std::vector<T>& values)
    {
        statistics<T> result;
        T total = T{0};

        if (values.empty())
            return result;

        result.min = result.max = values.front();
        for (auto x : values) {
            result.min = std::min(result.min, x);
            result.max = std::max(result.max, x);
            total += x;
        }

        result.avg = total / values.size();

        return result;
    }

} // namespace


clock_item::clock_item() :
    button_item{"Clock"}
{}


std::unique_ptr<clock_item>
clock_item::create()
{
    return std::make_unique<clock_item>();
}


void
clock_item::on_started()
{
    try {
        status_msg = "";
        run();
        update_status_msg();
    }
    catch (std::exception& e) {
        status_msg = e.what();
    }
    current_state = state::finished;
}


void
clock_item::update_status_msg()
{
    now_str = core::local_clock_to_string();
    status_msg = now_str + diff_str;
}


/*
 * Note: this code is very similar to core::run(), but runs in a single thread.
 */
void
clock_item::run()
{
    using std::to_string;
    using time_utils::seconds_to_human;

    for (auto& [key, value] : server_infos) {
        value.name->text.clear();
        value.correction->text.clear();
        value.latency->text.clear();
    }

    auto servers = utils::split(cfg::server, " \t,;");

    net::addrinfo::hints opts{ .type = net::socket::type::udp };

    dbl_seconds total = 0s;
    unsigned num_values = 0;

    for (const auto& server : servers) {
        auto& si = server_infos.at(server);
        try {
            auto infos = net::addrinfo::lookup(server, "123", opts);

            si.name->text = to_string(infos.size())
                + (infos.size() > 1 ? " addresses."s : " address."s);

            std::vector<dbl_seconds> server_corrections;
            std::vector<dbl_seconds> server_latencies;
            unsigned errors = 0;

            for (const auto& info : infos) {
                try {
                    auto [correction, latency] = core::ntp_query({}, info.addr);
                    server_corrections.push_back(correction);
                    server_latencies.push_back(latency);
                    total += correction;
                    ++num_values;
                    logger::printf("%s (%s): correction = %s, latency = %s\n",
                                   server.c_str(),
                                   to_string(info.addr).c_str(),
                                   seconds_to_human(correction, true).c_str(),
                                   seconds_to_human(latency).c_str());
                }
                catch (std::exception& e) {
                    ++errors;
                    logger::printf("Error: %s\n", e.what());
                }
            }

            if (errors)
                si.name->text += " "s + to_string(errors)
                    + (errors > 1 ? " errors."s : " error."s);
            if (!server_corrections.empty()) {
                auto corr_stats = get_statistics(server_corrections);
                si.correction->text = "min = "s + seconds_to_human(corr_stats.min, true)
                    + ", max = "s + seconds_to_human(corr_stats.max, true)
                    + ", avg = "s + seconds_to_human(corr_stats.avg, true);
                auto late_stats = get_statistics(server_latencies);
                si.latency->text = "min = "s + seconds_to_human(late_stats.min)
                    + ", max = "s + seconds_to_human(late_stats.max)
                    + ", avg = "s + seconds_to_human(late_stats.avg);
            } else {
                si.correction->text = "No data.";
                si.latency->text = "No data.";
            }
        }
        catch (std::exception& e) {
            si.name->text = e.what();
        }
    }

    if (num_values) {
        dbl_seconds avg = total / num_values;
        diff_str = ", needs "s + seconds_to_human(avg, true);
    } else
        diff_str = "";
}
