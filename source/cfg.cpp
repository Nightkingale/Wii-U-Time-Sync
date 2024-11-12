/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 * Copyright (C) 2024  Nightkingale
 *
 * SPDX-License-Identifier: MIT
 */

#include <wupsxx/bool_item.hpp>
#include <wupsxx/category.hpp>
#include <wupsxx/duration_items.hpp>
#include <wupsxx/init.hpp>
#include <wupsxx/logger.hpp>
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

using namespace std::literals;
using namespace wups::config;
namespace logger = wups::logger;


namespace cfg {

    namespace keys {
        const char* auto_tz         = "auto_tz";
        const char* msg_duration    = "msg_duration";
        const char* notify          = "notify";
        const char* server          = "server";
        const char* sync_on_boot    = "sync_on_boot";
        const char* sync_on_changes = "sync_on_changes";
        const char* threads         = "threads";
        const char* timeout         = "timeout";
        const char* tolerance       = "tolerance";
        const char* tz_service      = "tz_service";
        const char* utc_offset      = "utc_offset";
    }


    namespace labels {
        const char* auto_tz         = "   └ Auto Update Time Zone";
        const char* msg_duration    = " └ Notification Duration";
        const char* notify          = "Show Notifications";
        const char* server          = "NTP Servers";
        const char* sync_on_boot    = "Synchronize On Boot";
        const char* sync_on_changes = "Synchronize After Changing Configuration";
        const char* threads         = "Background Threads";
        const char* timeout         = "Timeout";
        const char* tolerance       = "Tolerance";
        const char* tz_service      = " └ Detect Time Zone";
        const char* utc_offset      = "Time Offset (UTC)";
    }


    namespace defaults {
        const bool         auto_tz         = false;
        const seconds      msg_duration    = 5s;
        const int          notify          = 0;
        const std::string  server          = "pool.ntp.org";
        const bool         sync_on_boot    = false;
        const bool         sync_on_changes = true;
        const int          threads         = 4;
        const seconds      timeout         = 5s;
        const milliseconds tolerance       = 500ms;
        const int          tz_service      = 0;
        const minutes      utc_offset      = 0min;
    }


    bool         auto_tz         = defaults::auto_tz;
    seconds      msg_duration    = defaults::msg_duration;
    int          notify          = defaults::notify;
    std::string  server          = defaults::server;
    bool         sync_on_boot    = defaults::sync_on_boot;
    bool         sync_on_changes = defaults::sync_on_changes;
    int          threads         = defaults::threads;
    seconds      timeout         = defaults::timeout;
    milliseconds tolerance       = defaults::tolerance;
    int          tz_service      = defaults::tz_service;
    minutes      utc_offset      = 0min;


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
        previous::auto_tz = auto_tz;
        previous::tolerance = tolerance;
        previous::tz_service = tz_service;
        previous::utc_offset = utc_offset;
    }


    bool
    important_vars_changed()
    {
        return previous::auto_tz != auto_tz
            || previous::tolerance != tolerance
            || previous::tz_service != tz_service
            || previous::utc_offset != utc_offset;
    }


    category
    make_config_screen()
    {
        category cat{"Configuration"};

        cat.add(bool_item::create(cfg::labels::sync_on_boot,
                                  cfg::sync_on_boot,
                                  cfg::defaults::sync_on_boot,
                                  "on", "off"));

        cat.add(bool_item::create(cfg::labels::sync_on_changes,
                                  cfg::sync_on_changes,
                                  cfg::defaults::sync_on_changes,
                                  "on", "off"));

        cat.add(verbosity_item::create(cfg::labels::notify,
                                       cfg::notify,
                                       cfg::defaults::notify));

        cat.add(seconds_item::create(cfg::labels::msg_duration,
                                     cfg::msg_duration,
                                     cfg::defaults::msg_duration,
                                     1s, 30s, 5s));

        cat.add(time_zone_offset_item::create(cfg::labels::utc_offset,
                                              cfg::utc_offset,
                                              cfg::defaults::utc_offset));

        cat.add(time_zone_query_item::create(cfg::labels::tz_service,
                                             cfg::tz_service,
                                             cfg::defaults::tz_service));

        cat.add(bool_item::create(cfg::labels::auto_tz,
                                  cfg::auto_tz,
                                  cfg::defaults::auto_tz,
                                  "on", "off"));

        cat.add(seconds_item::create(cfg::labels::timeout,
                                     cfg::timeout,
                                     cfg::defaults::timeout,
                                     1s, 10s, 5s));

        cat.add(milliseconds_item::create(cfg::labels::tolerance,
                                          cfg::tolerance,
                                          cfg::defaults::tolerance,
                                          0ms, 5000ms, 100ms));

        cat.add(int_item::create(cfg::labels::threads,
                                 cfg::threads,
                                 cfg::defaults::threads,
                                 0, 8, 2));

        // show current NTP server address, no way to change it.
        cat.add(text_item::create(cfg::labels::server,
                                  cfg::server));

        return cat;
    }


    void
    menu_open(category& root)
    {
        logger::initialize(PLUGIN_NAME);

        cfg::reload();

        root.add(make_config_screen());
        root.add(make_preview_screen());
        root.add(synchronize_item::create());

        save_important_vars();
    }


    void
    menu_close()
    {
        if (cfg::sync_on_changes && important_vars_changed()) {
            core::background::stop();
            core::background::run();
        }

        cfg::save();

        logger::finalize();
    }


    void migrate_old_config();


    void
    init()
    {
        try {
            wups::config::init(PLUGIN_NAME,
                               menu_open,
                               menu_close);
            cfg::load();
            cfg::migrate_old_config();
        }
        catch (std::exception& e) {
            logger::printf("Init error: %s\n", e.what());
        }
    }


    void
    load()
    {
        try {
#define LOAD(x) wups::storage::load_or_init(keys::x, x, defaults::x)
            LOAD(auto_tz);
            LOAD(msg_duration);
            LOAD(notify);
            LOAD(server);
            LOAD(sync_on_boot);
            LOAD(sync_on_changes);
            LOAD(threads);
            LOAD(timeout);
            LOAD(tolerance);
            LOAD(tz_service);
            LOAD(utc_offset);
#undef LOAD
        }
        catch (std::exception& e) {
            logger::printf("Error loading config: %s\n", e.what());
        }
    }


    void
    reload()
    {
        try {
            wups::storage::reload();
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
#define STORE(x) wups::storage::store(keys::x, x)
            STORE(auto_tz);
            STORE(msg_duration);
            STORE(notify);
            STORE(server);
            STORE(sync_on_boot);
            STORE(sync_on_changes);
            STORE(threads);
            STORE(timeout);
            STORE(tolerance);
            STORE(tz_service);
            STORE(utc_offset);
#undef STORE
            wups::storage::save();
        }
        catch (std::exception& e) {
            logger::printf("Error saving config: %s\n", e.what());
        }
    }


    void
    migrate_old_config()
    {
        using std::to_string;

        // check for leftovers from old versions
        auto hrs = wups::storage::load<hours>("hours");
        auto min = wups::storage::load<minutes>("minutes");
        if (hrs || min) {
            hours h = hrs.value_or(0h);
            minutes m = min.value_or(0min);
            minutes offset = h + m;
            set_and_store_utc_offset(offset);
            WUPSStorageAPI::DeleteItem("hours");
            WUPSStorageAPI::DeleteItem("minutes");
            save();
            logger::printf("Migrated old config: hours=%s, minutes=%s -> utc_offset=%s.\n",
                           to_string(h.count()).c_str(),
                           to_string(m.count()).c_str(),
                           time_utils::tz_offset_to_string(utc_offset).c_str());
        }
        auto sync = wups::storage::load<bool>("sync");
        if (sync) {
            WUPSStorageAPI::DeleteItem("sync");
            sync_on_boot = *sync;
            save();
            logger::printf("Migrated old config: sync=%s -> sync_on_boot=%s\n",
                           (*sync ? "true" : "false"),
                           (sync_on_boot ? "true" : "false"));
        }
    }


    void
    set_and_store_utc_offset(minutes offset)
    {
        logger::guard guard(PLUGIN_NAME);
        /*
         * Normally, `utc_offset` is saved when closing the config menu.
         * If auto_tz is enabled, it will be updated and saved outside the config menu.
         */
        utc_offset = offset;
        try {
            wups::storage::store(keys::utc_offset, utc_offset);
            wups::storage::save();
        }
        catch (std::exception& e) {
            logger::printf("Error storing utc_offset: %s\n", e.what());
        }
    }

} // namespace cfg
