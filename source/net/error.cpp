/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include "net/error.hpp"


namespace net {

    error::error(int code, const std::string& msg) :
        std::system_error{std::make_error_code(std::errc{code}), msg}
    {}


    error::error(int code) :
        std::system_error{std::make_error_code(std::errc{code})}
    {}

} // namespace std
