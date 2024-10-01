/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include <iterator>             // distance()
#include <stdexcept>            // logic_error, runtime_error

#include <nn/ac.h>

#include "utils.hpp"

#include "http_client.hpp"


using namespace std::literals;

using std::logic_error;
using std::runtime_error;


namespace utils {

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


    namespace {

        /*
         * Split CSV line:
         *   - Separator is always `,`.
         *   - Separators inside quotes (`"` or `'`) are ignored.
         *   - Don't discard empty tokens.
         */
        std::vector<std::string>
        csv_split(const std::string& input)
        {
            using std::string;

            std::vector<string> result;

            string::size_type start = 0;
            for (string::size_type i = 0; i < input.size(); ++i) {
                char c = input[i];
                if (c == '"' || c == '\'') {
                    // jump to the closing quote
                    i = input.find(c, i + 1);
                    if (i == string::npos)
                        break; // if there's no closing quote, it's bad input
                } else if (c == ',') {
                    result.push_back(input.substr(start, i - start));
                    start = i + 1;
                }
            }
            // whatever remains from `start` to the end is the last token
            result.push_back(input.substr(start));
            return result;
        }

    } // namespace


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


    namespace {
        constexpr int num_tz_services = 3;
    }


    int
    get_num_tz_services()
    {
        return num_tz_services;
    }


    const char*
    get_tz_service_name(int idx)
    {
        switch (idx) {
        case 0:
            return "http://ip-api.com";
        case 1:
            return "https://ipwho.is";
        case 2:
            return "https://ipapi.co";
        default:
            throw logic_error{"Invalid tz service."};
        }
    }


    std::pair<std::string, std::chrono::minutes>
    fetch_timezone(int idx)
    {
        const char* service = get_tz_service_name(idx);

        static const char* urls[num_tz_services] = {
            "http://ip-api.com/csv/?fields=timezone,offset",
            "https://ipwho.is/?fields=timezone.id,timezone.offset&output=csv",
            "https://ipapi.co/csv"
        };

        network_guard net_guard;

        std::string response = http::get(urls[idx]);

        switch (idx) {
        case 0: // http://ip-api.com
        case 1: // https://ipwho.is
            {
                auto tokens = csv_split(response);
                if (size(tokens) != 2)
                    throw runtime_error{"Could not parse response from "s + service};
                std::string name = tokens[0];
                auto offset = std::chrono::seconds{std::stoi(tokens[1])};
                return {name, duration_cast<std::chrono::minutes>(offset)};
            }

        case 2: // https://ipapi.co
            {
                // This returns a CSV header and CSV fields in two rows, gotta find
                // indexes for "timezone" and "utc_offset" fields. The "utc_offset" is
                // returned as +HHMM, not seconds.
                auto lines = split(response, "\r\n");
                if (size(lines) != 2)
                    throw runtime_error{"Could not parse response from "s + service};

                auto keys = csv_split(lines[0]);
                auto values = csv_split(lines[1]);
                if (size(keys) != size(values))
                    throw runtime_error{"Incoherent response from "s + service};

                auto tz_it = std::ranges::find(keys, "timezone");
                auto offset_it = std::ranges::find(keys, "utc_offset");
                if (tz_it == keys.end() || offset_it == keys.end())
                    throw runtime_error{"Could not find timezone or utc_offset fields"
                                        " in response."};

                auto tz_idx = std::distance(keys.begin(), tz_it);;
                auto offset_idx = std::distance(keys.begin(), offset_it);

                std::string name = values[tz_idx];
                std::string hhmm = values[offset_idx];
                if (empty(hhmm))
                    throw runtime_error{"Invalid UTC offset string."};

                char sign = hhmm[0];
                std::string hh = hhmm.substr(1, 2);
                std::string mm = hhmm.substr(3, 2);
                int h = std::stoi(hh);
                int m = std::stoi(mm);
                int total = h * 60 + m;
                if (sign == '-')
                    total = -total;
                return {name, std::chrono::minutes{total}};
            }

        default:
            throw logic_error{"Invalid tz service."};
        }

    }


    network_guard::init_guard::init_guard()
    {
        if (!nn::ac::Initialize())
            throw runtime_error{"Network error (nn::ac::Initialize() failed)"};
    }


    network_guard::init_guard::~init_guard()
    {
        nn::ac::Finalize();
    }


    network_guard::connect_guard::connect_guard()
    {
        if (!nn::ac::Connect())
            throw runtime_error{"Network error (nn::ac::Connect() failed)"};
    }


    network_guard::connect_guard::~connect_guard()
    {
        nn::ac::Close();
    }

} // namespace utils
