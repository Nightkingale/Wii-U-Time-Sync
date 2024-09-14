/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include "http_client.hpp"

#include "curl.hpp"


namespace http {

    std::string
    get(const std::string& url)
    {
        curl::global guard;

        curl::handle handle;

        handle.set_useragent(PLUGIN_NAME "/" PLUGIN_VERSION " (Wii U; Aroma)");
        handle.set_followlocation(true);
        handle.set_url(url);

        handle.perform();

        return handle.result;
    }

} // namespace http
