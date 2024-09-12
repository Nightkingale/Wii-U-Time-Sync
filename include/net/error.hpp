/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef NET_ERROR_HPP
#define NET_ERROR_HPP

#include <string>
#include <system_error>


namespace net {

    struct error : std::system_error {

        error(int code, const std::string& msg);

        error(int code);

    };

} // namespace net

#endif
