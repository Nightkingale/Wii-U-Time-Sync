/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <atomic>
#include <chrono>
#include <cstddef>              // size_t
#include <string>
#include <utility>              // pair<>
#include <vector>


namespace utils {

    /**
     * Split input string into tokens, according to separators.
     *
     * If max_tokens is not zero, only up to max_tokens will be generated; the last token
     * will be the remaining of the string.
     */
    std::vector<std::string>
    split(const std::string& input,
          const std::string& separators,
          std::size_t max_tokens = 0);


    // RAII type to ensure a function is never executed in parallel.
    struct exec_guard {

        std::atomic_bool& flag;
        bool guarded; // when false, the function is already executing in some thread.

        exec_guard(std::atomic<bool>& f);

        ~exec_guard();

    };


    int
    get_num_tz_services();


    const char*
    get_tz_service_name(int idx);


    std::pair<std::string,
              std::chrono::minutes>
    fetch_timezone(int idx);


    // RAII class to ensure network is working.
    // It blocks until the network is available, of throws std::runtime_error.
    class network_guard {

        struct init_guard {
            init_guard();
            ~init_guard();
        };

        struct connect_guard {
            connect_guard();
            ~connect_guard();
        };

        init_guard init;
        connect_guard conn;

    };

} // namespace utils

#endif
