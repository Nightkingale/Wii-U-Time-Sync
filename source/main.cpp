// SPDX-License-Identifier: MIT

// standard headers
#include <memory>               // make_unique()
#include <thread>

// WUT/WUPS headers
#include <notifications/notifications.h>
#include <wups.h>
#include <whb/log_udp.h>

// local headers
#include "../include/cfg.hpp"
#include "../include/config_screen.hpp"
#include "../include/preview_screen.hpp"
#include "../include/core.hpp"
#include "../include/wupsxx/config.hpp"


// Important plugin information.
WUPS_PLUGIN_NAME(PLUGIN_NAME);
WUPS_PLUGIN_DESCRIPTION(PLUGIN_DESCRIPTION);
WUPS_PLUGIN_VERSION(PLUGIN_VERSION);
WUPS_PLUGIN_AUTHOR(PLUGIN_AUTHOR);
WUPS_PLUGIN_LICENSE(PLUGIN_LICENSE);

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE(PLUGIN_NAME);


INITIALIZE_PLUGIN()
{
    WHBLogUdpInit();
    NotificationModule_InitLibrary(); // Set up for notifications.

    // Check if the plugin's settings have been saved before.
    if (WUPS_OpenStorage() == WUPS_STORAGE_ERROR_SUCCESS) {
        cfg::load();
        WUPS_CloseStorage();
    }

    if (cfg::sync)
        core::sync_clock(); // Update clock when plugin is loaded.
}


WUPS_GET_CONFIG()
{
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS)
        return 0;

    try {
        auto root = std::make_unique<wups::config>(PLUGIN_NAME);

        root->add(std::make_unique<config_screen>());
        root->add(std::make_unique<preview_screen>());

        return root.release()->handle;
    }
    catch (...) {
        return 0;
    }
}


WUPS_CONFIG_CLOSED()
{
    std::jthread update_time_thread(core::sync_clock);
    update_time_thread.detach(); // Update time when settings are closed.

    WUPS_CloseStorage(); // Save all changes.
}
