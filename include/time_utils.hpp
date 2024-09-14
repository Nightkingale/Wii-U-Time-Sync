/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TIME_UTILS_HPP
#define TIME_UTILS_HPP

#include <chrono>
#include <string>


namespace time_utils {

    // Type-safe way to pass seconds around as double
    using dbl_seconds = std::chrono::duration<double>;


    // Generate time duration strings for humans.
    std::string seconds_to_human(dbl_seconds s, bool show_positive = false);


    std::string tz_offset_to_string(std::chrono::minutes offset);

}


#endif
