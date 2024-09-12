/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 * Copyright (C) 2024  Nightkingale
 *
 * SPDX-License-Identifier: MIT
 */

#include <exception>
#include <thread>

#include <wups.h>

#include <wupsxx/logger.hpp>

#include "cfg.hpp"
#include "core.hpp"
#include "notify.hpp"


// Important plugin information.
WUPS_PLUGIN_NAME(PLUGIN_NAME);
WUPS_PLUGIN_VERSION(PLUGIN_VERSION);
WUPS_PLUGIN_DESCRIPTION("A plugin that synchronizes the system clock to the Internet.");
WUPS_PLUGIN_AUTHOR("Nightkingale, Daniel K. O.");
WUPS_PLUGIN_LICENSE("MIT");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE(PLUGIN_NAME);


INITIALIZE_PLUGIN()
{
    wups::logger::guard guard{PLUGIN_NAME};

    cfg::init();

    if (cfg::sync_on_boot) {
        std::jthread t{
            [](std::stop_token token)
            {
                wups::logger::guard lguard{PLUGIN_NAME};
                notify::guard nguard;
                try {
                    core::run(token, false);
                }
                catch (std::exception& e) {
                    notify::error(notify::level::normal, e.what());
                }
            }
        };
        t.detach();
    }
}


DEINITIALIZE_PLUGIN()
{
    // TODO: should clean up any worker thread
}
