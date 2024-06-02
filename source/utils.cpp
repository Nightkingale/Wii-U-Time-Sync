// SPDX-License-Identifier: MIT

#include <stdexcept>            // runtime_error

#include "utils.hpp"

#include "http_client.hpp"


using namespace std::literals;


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

} // namespace utils
