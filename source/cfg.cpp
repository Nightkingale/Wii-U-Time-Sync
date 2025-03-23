/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2025  Daniel K. O.
 * Copyright (C) 2024  Nightkingale
 *
 * SPDX-License-Identifier: MIT
 */

#include <vector>

#include <wupsxx/bool_item.hpp>
#include <wupsxx/category.hpp>
#include <wupsxx/duration_items.hpp>
#include <wupsxx/init.hpp>
#include <wupsxx/logger.hpp>
#include <wupsxx/option.hpp>
#include <wupsxx/storage.hpp>
#include <wupsxx/text_item.hpp>

#include "cfg.hpp"

#include "core.hpp"
#include "notify.hpp"
#include "preview_screen.hpp"
#include "synchronize_item.hpp"
#include "time_utils.hpp"
#include "time_zone_offset_item.hpp"
#include "time_zone_query_item.hpp"
#include "utils.hpp"
#include "verbosity_item.hpp"


using std::chrono::hours;
using std::chrono::milliseconds;
using std::chrono::minutes;
using std::chrono::seconds;

using wups::category;

using namespace std::literals;
namespace logger = wups::logger;


namespace cfg {

    WUPSXX_OPTION("Synchronize On Boot",
                  bool, sync_on_boot, true);

    WUPSXX_OPTION("Synchronize After Changing Configuration",
                  bool, sync_on_changes, true);

    WUPSXX_OPTION("Show Notifications",
                  int, notify, 0, 0, 2);

    WUPSXX_OPTION("  └ Notification Duration",
                  seconds, msg_duration, 5s, 0s, 15s);

    WUPSXX_OPTION("Time Offset (UTC)",
                  minutes, utc_offset, 0min, -12h, 14h);

    WUPSXX_OPTION("  └ Detect Time Zone",
                  int, tz_service, 0, 0, utils::get_num_tz_services());

    WUPSXX_OPTION("    └ Auto Update Time Zone",
                  bool, auto_tz, false);

    WUPSXX_OPTION("Timeout",
                  seconds, timeout, 5s, 1s, 10s);

    WUPSXX_OPTION("Tolerance",
                  milliseconds, tolerance, 1000ms, 0ms, 10s);

    WUPSXX_OPTION("Background Threads",
                  int, threads, 4, 0, 4);

    WUPSXX_OPTION("NTP servers",
                  std::string, server, "pool.ntp.org");


    std::vector<wups::option_base*> all_options = {
        &sync_on_boot,
        &sync_on_changes,
        &notify,
        &msg_duration,
        &utc_offset,
        &tz_service,
        &auto_tz,
        &timeout,
        &tolerance,
        &threads,
        &server,
    };


    // variables that, if changed, may affect the sync
    namespace previous {
        bool         auto_tz;
        milliseconds tolerance;
        int          tz_service;
        minutes      utc_offset;
    }


    void
    save_important_vars()
    {
        previous::auto_tz    = auto_tz.value;
        previous::tolerance  = tolerance.value;
        previous::tz_service = tz_service.value;
        previous::utc_offset = utc_offset.value;
    }


    bool
    important_vars_changed()
    {
        return previous::auto_tz != auto_tz.value
            || previous::tolerance != tolerance.value
            || previous::tz_service != tz_service.value
            || previous::utc_offset != utc_offset.value;
    }


    category
    make_config_screen()
    {
        using wups::make_item;

        category cat{"Configuration"};

        cat.add(make_item(sync_on_boot, "on", "off"));

        cat.add(make_item(sync_on_changes, "on", "off"));

        cat.add(verbosity_item::create(notify));

        cat.add(make_item(msg_duration));

        cat.add(time_zone_offset_item::create(utc_offset));

        cat.add(time_zone_query_item::create(tz_service));

        cat.add(make_item(auto_tz, "on", "off"));

        cat.add(make_item(timeout));

        cat.add(make_item(tolerance, 500ms, 100ms));

        cat.add(make_item(threads));

        // show current NTP server address, no way to change it.
        cat.add(make_item(server.label, server.value));

        return cat;
    }


    void
    menu_open(category& root)
    {
        // Keep logger active until the menu closes
        logger::initialize();

        reload();

        root.add(make_config_screen());
        root.add(make_preview_screen());
        root.add(synchronize_item::create());

        save_important_vars();
    }


    void
    menu_close()
    {
        logger::guard guard; // keep logger active until the function ends
        logger::finalize(); // clean up the initialize() from menu_open()

        notify::set_max_level(static_cast<notify::level>(notify.value));

        if (sync_on_changes.value && important_vars_changed()) {
            core::background::stop();
            core::background::run();
        }

        save();
    }


    void migrate_old_config();


    void
    init()
    {
        try {
            wups::init(PLUGIN_NAME,
                       menu_open,
                       menu_close);
            load();
            migrate_old_config();
        }
        catch (std::exception& e) {
            logger::printf("Init error: %s\n", e.what());
        }
    }


    void
    load()
    {
        for (auto& opt : all_options)
            opt->load();
        notify::set_max_level(static_cast<notify::level>(notify.value));
    }


    void
    reload()
    {
        try {
            wups::reload();
            load();
        }
        catch (std::exception& e) {
            logger::printf("Error reloading config: %s\n", e.what());
        }
    }


    void
    save()
    {
        try {
            for (const auto& opt : all_options)
                opt->store();
            wups::save();
        }
        catch (std::exception& e) {
            logger::printf("Error saving config: %s\n", e.what());
        }
    }


    void
    migrate_old_config()
    {
        // check for leftovers from old versions
        using std::to_string;
        using wups::to_string;

        hours old_hours = 0h;
        minutes old_minutes = 0min;
        if (wups::load("hours", old_hours) || wups::load("minutes", old_minutes)) {
            minutes offset = old_hours + old_minutes;
            set_and_store_utc_offset(offset);
            WUPSStorageAPI::DeleteItem("hours");
            WUPSStorageAPI::DeleteItem("minutes");
            save();
            logger::printf("Migrated old config: hours=%s, minutes=%s -> utc_offset=%s.\n",
                           to_string(old_hours).c_str(),
                           to_string(old_minutes).c_str(),
                           time_utils::tz_offset_to_string(utc_offset.value).c_str());
        }

        bool old_sync = false;
        if (wups::load("sync", old_sync)) {
            WUPSStorageAPI::DeleteItem("sync");
            sync_on_boot.value = old_sync;
            save();
            logger::printf("Migrated old config: sync=%s -> sync_on_boot=%s\n",
                           (old_sync ? "true" : "false"),
                           (sync_on_boot.value ? "true" : "false"));
        }
    }


    void
    set_and_store_utc_offset(minutes offset)
    {
        logger::guard guard;
        /*
         * Normally, `utc_offset` is saved when closing the config menu.
         * If auto_tz is enabled, it will be updated and saved outside the config menu.
         */
        try {
            utc_offset.value = offset;
            utc_offset.store();
            wups::save();
        }
        catch (std::exception& e) {
            logger::printf("Error storing utc_offset: %s\n", e.what());
        }
    }

} // namespace cfg
