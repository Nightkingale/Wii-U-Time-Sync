/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include <atomic>

#include <notifications/notifications.h>

#include <wupsxx/logger.hpp>

#include "notify.hpp"

#include "cfg.hpp"


namespace logger = wups::logger;


namespace notify {

    std::atomic_uint refs = 0;


    void
    initialize()
    {
        // don't initialize again if refs was already positive
        if (refs++)
            return;

        NotificationModule_InitLibrary();
    }


    void
    finalize()
    {
        if (!refs)
            return;

        // don't finalize if refs is still positive
        if (--refs)
            return;

        NotificationModule_DeInitLibrary();
    }


    void
    error(level lvl, const std::string& arg)
    {
        logger::printf("ERROR: %s\n", arg.c_str());

        if (static_cast<int>(lvl) > cfg::notify)
            return;

        std::string msg = "[" PLUGIN_NAME "] " + arg;
        NotificationModule_AddErrorNotificationEx(msg.c_str(),
                                                  cfg::msg_duration.count(),
                                                  1,
                                                  {255, 255, 255, 255},
                                                  {160, 32, 32, 255},
                                                  nullptr,
                                                  nullptr,
                                                  true);
    }


    void
    info(level lvl, const std::string& arg)
    {
        logger::printf("INFO: %s\n", arg.c_str());

        if (static_cast<int>(lvl) > cfg::notify)
            return;

        std::string msg = "[" PLUGIN_NAME "] " + arg;
        NotificationModule_AddInfoNotificationEx(msg.c_str(),
                                                 cfg::msg_duration.count(),
                                                 {255, 255, 255, 255},
                                                 {32, 32, 160, 255},
                                                 nullptr,
                                                 nullptr,
                                                 true);
    }


    void
    success(level lvl, const std::string& arg)
    {
        logger::printf("SUCCESS: %s\n", arg.c_str());

        if (static_cast<int>(lvl) > cfg::notify)
            return;

        std::string msg = "[" PLUGIN_NAME "] " + arg;
        NotificationModule_AddInfoNotificationEx(msg.c_str(),
                                                 cfg::msg_duration.count(),
                                                 {255, 255, 255, 255},
                                                 {32, 160, 32, 255},
                                                 nullptr,
                                                 nullptr,
                                                 true);
    }


    guard::guard(bool init) :
        must_finalize{init}
    {
        if (init)
            initialize();
    }


    guard::~guard()
    {
        if (must_finalize)
            finalize();
    }


    void
    guard::release()
    {
        must_finalize = false;
    }


}
