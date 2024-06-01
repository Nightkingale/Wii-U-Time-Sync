// SPDX-License-Identifier: MIT

#include <cmath>                // abs()
#include <cstdio>               // snprintf()
#include <stdexcept>            // runtime_error

#include "utils.hpp"

#include "http_client.hpp"


using namespace std::literals;


namespace utils {

    std::string
    seconds_to_human(double s, bool show_positive)
    {
        char buf[64];

        if (std::abs(s) < 2) // less than 2 seconds
            std::snprintf(buf, sizeof buf, "%.1f ms", 1000 * s);
        else if (std::abs(s) < 2 * 60) // less than 2 minutes
            std::snprintf(buf, sizeof buf, "%.1f s", s);
        else if (std::abs(s) < 2 * 60 * 60) // less than 2 hours
            std::snprintf(buf, sizeof buf, "%.1f min", s / 60);
        else if (std::abs(s) < 2 * 24 * 60 * 60) // less than 2 days
            std::snprintf(buf, sizeof buf, "%.1f hrs", s / (60 * 60));
        else
            std::snprintf(buf, sizeof buf, "%.1f days", s / (24 * 60 * 60));

        std::string result = buf;

        if (show_positive && s > 0)
            result = "+" + result;

        return result;
    }


    std::vector<std::string>
    split(const std::string& input,
          const std::string& separators,
          std::size_t max_tokens)
    {
        using std::string;

        std::vector<string> result;

        string::size_type start = input.find_first_not_of(separators);
        while (start != string::npos) {

            // if we can only include one more token
            if (max_tokens && result.size() + 1 == max_tokens) {
                // the last token will be the remaining of the input
                result.push_back(input.substr(start));
                break;
            }

            auto finish = input.find_first_of(separators, start);
            result.push_back(input.substr(start, finish - start));
            start = input.find_first_not_of(separators, finish);
        }

        return result;
    }


    exec_guard::exec_guard(std::atomic<bool>& f) :
        flag(f),
        guarded{false}
    {
        bool expected_flag = false;
        if (flag.compare_exchange_strong(expected_flag, true))
            guarded = true; // Exactly one thread can have the "guarded" flag as true.
    }


    exec_guard::~exec_guard()
    {
        if (guarded)
            flag = false;
    }


    std::pair<std::string, std::chrono::minutes>
    fetch_timezone()
    {
        std::string tz = http::get("http://ip-api.com/line/?fields=timezone,offset");
        auto tokens = utils::split(tz, " \r\n");
        if (tokens.size() != 2)
            throw std::runtime_error{"Could not parse response from \"ip-api.com\"."};

        int tz_offset_min = std::stoi(tokens[1]) / 60;
        return {tokens[0], std::chrono::minutes{tz_offset_min}};
    }


    std::string
    tz_offset_to_string(std::chrono::minutes offset)
    {
        char buf[32];
        char sign = offset < 0min ? '-' : '+';
        int hours = std::abs(offset.count() / 60);
        int minutes = std::abs(offset.count() % 60);
        std::snprintf(buf, sizeof buf, "%c%02d:%02d", sign, hours, minutes);
        return buf;
    }

} // namespace utils
