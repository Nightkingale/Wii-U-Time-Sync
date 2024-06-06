// SPDX-License-Identifier: MIT

#ifndef CFG_HPP
#define CFG_HPP

#include <chrono>
#include <string>


namespace cfg {

    namespace key {
        extern const char* auto_tz;
        extern const char* msg_duration;
        extern const char* notify;
        extern const char* server;
        extern const char* sync;
        extern const char* threads;
        extern const char* tolerance;
        extern const char* tz_service;
        extern const char* utc_offset;
    }


    namespace label {
        extern const char* auto_tz;
        extern const char* msg_duration;
        extern const char* notify;
        extern const char* server;
        extern const char* sync;
        extern const char* threads;
        extern const char* tolerance;
        extern const char* tz_service;
        extern const char* utc_offset;
    }

    namespace defaults {
        extern const bool                      auto_tz;
        extern const std::chrono::seconds      msg_duration;
        extern const int                       notify;
        extern const std::string               server;
        extern const bool                      sync;
        extern const int                       threads;
        extern const std::chrono::milliseconds tolerance;
        extern const int                       tz_service;
    }


    extern bool                      auto_tz;
    extern std::chrono::seconds      msg_duration;
    extern int                       notify;
    extern std::string               server;
    extern bool                      sync;
    extern int                       threads;
    extern std::chrono::milliseconds tolerance;
    extern int                       tz_service;
    extern std::chrono::minutes      utc_offset;


    void load();
    void reload();
    void save();
    void migrate_old_config();

    void set_and_store_utc_offset(std::chrono::minutes tz_offset);

} // namespace cfg

#endif
