/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstdarg>
#include <string>

#include <wupsxx/logger.hpp>
#include <wupsxx/notify.hpp>

#include "notify.hpp"


using namespace std::literals;

namespace logger = wups::logger;


namespace notify {

    level max_level = level::quiet;


    void
    initialize()
    {
        wups::notify::initialize(PLUGIN_NAME);

        wups::notify::info::set_text_color(255, 255, 255, 255);
        wups::notify::info::set_bg_color(32, 32, 160, 255);

        wups::notify::error::set_text_color(255, 255, 255, 255);
        wups::notify::error::set_bg_color(160, 32, 32, 255);
    }


    void
    finalize()
    {
        wups::notify::finalize();
    }


    void
    set_max_level(level lvl)
        noexcept
    {
        max_level = lvl;
    }


    void
    set_duration(std::chrono::milliseconds dur)
        noexcept
    {
        wups::notify::info::set_duration(dur);
        wups::notify::error::set_duration(dur);
    }


    __attribute__(( __format__ (__printf__, 2, 3)))
    void
    error(level lvl,
          const char* fmt,
          ...)
        noexcept
    {
        std::va_list logger_args;
        va_start(logger_args, fmt);
        try {
            std::string logger_fmt = "ERROR: "s + fmt + "\n"s;
            logger::vprintf(logger_fmt.data(), logger_args);
        }
        catch (...) {}
        va_end(logger_args);

        if (lvl > max_level)
            return;

        std::va_list notify_args;
        va_start(notify_args, fmt);
        try {
            wups::notify::error::vshow(fmt, notify_args);
        }
        catch (std::exception& e) {
            logger::printf("notification error: %s\n", e.what());
        }
        va_end(notify_args);
    }


    __attribute__(( __format__ (__printf__, 2, 3)))
    void
    info(level lvl,
         const char* fmt,
         ...)
        noexcept
    {
        std::va_list logger_args;
        va_start(logger_args, fmt);
        try {
            std::string logger_fmt = "INFO: "s + fmt + "\n"s;
            logger::vprintf(logger_fmt.data(), logger_args);
        }
        catch (...) {}
        va_end(logger_args);

        if (lvl > max_level)
            return;

        std::va_list notify_args;
        va_start(notify_args, fmt);
        try {
            wups::notify::info::vshow(fmt, notify_args);
        }
        catch (std::exception& e) {
            logger::printf("notification error: %s\n", e.what());
        }
        va_end(notify_args);
    }


    __attribute__(( __format__ (__printf__, 2, 3)))
    void
    success(level lvl,
            const char* fmt,
            ...)
        noexcept
    {
        std::va_list logger_args;
        va_start(logger_args, fmt);
        try {
            std::string logger_fmt = "SUCCESS: "s + fmt + "\n"s;
            logger::vprintf(logger_fmt.data(), logger_args);
        }
        catch (...) {}
        va_end(logger_args);

        if (lvl > max_level)
            return;

        std::va_list notify_args;
        va_start(notify_args, fmt);
        try {
            wups::notify::info::vshow(wups::color{255, 255, 255},
                                      wups::color{32, 160, 32},
                                      fmt,
                                      notify_args);
        }
        catch (std::exception& e) {
            logger::printf("notification error: %s\n", e.what());
        }
        va_end(notify_args);
    }

} // namespace notify
