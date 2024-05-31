// SPDX-License-Identifier: MIT

#include <cmath>                // fmax(), fmin()
#include <exception>
#include <vector>

#include "clock_item.hpp"

#include "cfg.hpp"
#include "core.hpp"
#include "logging.hpp"
#include "net/addrinfo.hpp"
#include "nintendo_glyphs.h"
#include "utils.hpp"


using wups::config::text_item;

using namespace std::literals;


namespace {

    struct statistics {
        double min = 0;
        double max = 0;
        double avg = 0;
    };


    statistics
    get_statistics(const std::vector<double>& values)
    {
        statistics result;
        double total = 0;

        if (values.empty())
            return result;

        result.min = result.max = values.front();
        for (auto x : values) {
            result.min = std::fmin(result.min, x);
            result.max = std::fmax(result.max, x);
            total += x;
        }

        result.avg = total / values.size();

        return result;
    }

}


clock_item::clock_item() :
    text_item{{}, "Clock (" NIN_GLYPH_BTN_A " to refresh)", "", 42}
{}


std::unique_ptr<clock_item>
clock_item::create()
{
    return std::make_unique<clock_item>();
}


void
clock_item::on_input(WUPSConfigSimplePadData input,
                     WUPS_CONFIG_SIMPLE_INPUT repeat)
{
    text_item::on_input(input, repeat);

    if (input.buttons_d & WUPS_CONFIG_BUTTON_A) {
        try {
            run();
        }
        catch (std::exception& e) {
            text = "Error: "s + e.what();
        }
    }

    refresh_now_str();
}


void
clock_item::refresh_now_str()
{
    now_str = core::local_clock_to_string();
    text = now_str + stats_str;
}


void
clock_item::run()
{
    using std::to_string;
    using utils::seconds_to_human;

    for (auto& [key, value] : server_infos) {
        value.name->text.clear();
        value.correction->text.clear();
        value.latency->text.clear();
    }

    auto servers = utils::split(cfg::server, " \t,;");

    net::addrinfo::hints opts{ .type = net::socket::type::udp };

    double total = 0;
    unsigned num_values = 0;

    for (const auto& server : servers) {
        auto& si = server_infos.at(server);
        try {
            auto infos = net::addrinfo::lookup(server, "123", opts);

            si.name->text = to_string(infos.size())
                + (infos.size() > 1 ? " addresses."s : " address."s);

            std::vector<double> server_corrections;
            std::vector<double> server_latencies;
            unsigned errors = 0;

            for (const auto& info : infos) {
                try {
                    auto [correction, latency] = core::ntp_query(info.addr);
                    server_corrections.push_back(correction);
                    server_latencies.push_back(latency);
                    total += correction;
                    ++num_values;
                    logging::printf("%s (%s): correction = %s, latency = %s",
                                    server.c_str(),
                                    to_string(info.addr).c_str(),
                                    seconds_to_human(correction, true).c_str(),
                                    seconds_to_human(latency).c_str());
                }
                catch (std::exception& e) {
                    ++errors;
                    logging::printf("Error: %s", e.what());
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
        double avg = total / num_values;
        stats_str = ", needs "s + seconds_to_human(avg, true);
    } else
        stats_str = "";

    refresh_now_str();
}
