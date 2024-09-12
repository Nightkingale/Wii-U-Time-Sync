/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include <string>


namespace http {

    std::string get(const std::string& url);

} // namespace http

#endif
