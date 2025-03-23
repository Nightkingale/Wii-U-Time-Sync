/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2025  Daniel K. O.
 * Copyright (C) 2024  Nightkingale
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef CFG_HPP
#define CFG_HPP

#include <chrono>
#include <string>

#include <wupsxx/option.hpp>

namespace cfg {

    extern wups::option<bool>                      auto_tz;
    extern wups::option<std::chrono::seconds>      msg_duration;
    extern wups::option<int>                       notify;
    extern wups::option<std::string>               server;
    extern wups::option<bool>                      sync_on_boot;
    extern wups::option<bool>                      sync_on_changes;
    extern wups::option<int>                       threads;
    extern wups::option<std::chrono::seconds>      timeout;
    extern wups::option<std::chrono::milliseconds> tolerance;
    extern wups::option<int>                       tz_service;
    extern wups::option<std::chrono::minutes>      utc_offset;

    void save_important_vars();

    void init() noexcept;

    void load();
    void reload();
    void save();

    void set_and_store_utc_offset(std::chrono::minutes tz_offset);

} // namespace cfg

#endif
