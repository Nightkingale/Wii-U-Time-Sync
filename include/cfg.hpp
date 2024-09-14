/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 * Copyright (C) 2024  Nightkingale
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef CFG_HPP
#define CFG_HPP

#include <chrono>
#include <string>


namespace cfg {

    extern bool                      auto_tz;
    extern std::chrono::seconds      msg_duration;
    extern int                       notify;
    extern std::string               server;
    extern bool                      sync_on_boot;
    extern bool                      sync_on_changes;
    extern int                       threads;
    extern std::chrono::seconds      timeout;
    extern std::chrono::milliseconds tolerance;
    extern int                       tz_service;
    extern std::chrono::minutes      utc_offset;

    void save_important_vars();

    void init();

    void load();
    void reload();
    void save();

    void set_and_store_utc_offset(std::chrono::minutes tz_offset);

} // namespace cfg

#endif
