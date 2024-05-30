// SPDX-License-Identifier: MIT

#include <exception>
#include <thread>

#include <wups.h>

#include "cfg.hpp"
#include "config_screen.hpp"
#include "core.hpp"
#include "logging.hpp"
#include "notify.hpp"
#include "preview_screen.hpp"
#include "wupsxx/category.hpp"


// Important plugin information.
WUPS_PLUGIN_NAME(PLUGIN_NAME);
WUPS_PLUGIN_DESCRIPTION(PLUGIN_DESCRIPTION);
WUPS_PLUGIN_VERSION(PLUGIN_VERSION);
WUPS_PLUGIN_AUTHOR(PLUGIN_AUTHOR);
WUPS_PLUGIN_LICENSE(PLUGIN_LICENSE);

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE(PLUGIN_NAME);


static WUPSConfigAPICallbackStatus open_config(WUPSConfigCategoryHandle root_handle);
static void close_config();


INITIALIZE_PLUGIN()
{
    logging::initialize();

    auto status = WUPSConfigAPI_Init({ .name = PLUGIN_NAME },
                                     open_config,
                                     close_config);
    if (status != WUPSCONFIG_API_RESULT_SUCCESS) {
        logging::printf("Init error: %s", WUPSConfigAPI_GetStatusStr(status));
        return;
    }

    cfg::load();
    cfg::migrate_old_config();

    if (cfg::sync)
        core::run(); // Update clock when plugin is loaded.
}


DEINITIALIZE_PLUGIN()
{
    logging::finalize();
}


static
WUPSConfigAPICallbackStatus
open_config(WUPSConfigCategoryHandle root_handle)
{
    try {
        cfg::reload();

        wups::config::category root{root_handle};

        root.add(make_config_screen());
        root.add(make_preview_screen());

        return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
    }
    catch (std::exception& e) {
        logging::printf("Error opening config: %s", e.what());
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }
}


static
void
close_config()
{
    cfg::save();

    // Update time when settings are closed.
    std::jthread update_time_thread{core::run};
    update_time_thread.detach();
}
