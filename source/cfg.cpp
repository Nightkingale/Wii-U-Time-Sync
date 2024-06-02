// SPDX-License-Identifier: MIT

#include "cfg.hpp"

#include "logging.hpp"
#include "time_utils.hpp"
#include "utils.hpp"
#include "wupsxx/storage.hpp"


using std::chrono::hours;
using std::chrono::milliseconds;
using std::chrono::minutes;
using std::chrono::seconds;

using namespace std::literals;


namespace cfg {

    namespace key {
        const char* auto_tz      = "auto_tz";
        const char* msg_duration = "msg_duration";
        const char* notify       = "notify";
        const char* server       = "server";
        const char* sync         = "sync";
        const char* threads      = "threads";
        const char* tolerance    = "tolerance";
        const char* utc_offset   = "utc_offset";
    }


    namespace label {
        const char* auto_tz      = "Auto Update Time Zone";
        const char* msg_duration = "Notification Duration";
        const char* notify       = "Show Notifications";
        const char* server       = "NTP Servers";
        const char* sync         = "Syncing Enabled";
        const char* threads      = "Background Threads";
        const char* tolerance    = "Tolerance (milliseconds)";
        const char* utc_offset   = "Time Offset (UTC)";
    }


    namespace defaults {
        const bool         auto_tz      = false;
        const seconds      msg_duration = 5s;
        const int          notify       = 0;
        const std::string  server       = "pool.ntp.org";
        const bool         sync         = false;
        const int          threads      = 4;
        const milliseconds tolerance    = 500ms;
    }


    bool         auto_tz      = defaults::auto_tz;
    seconds      msg_duration = defaults::msg_duration;
    int          notify       = defaults::notify;
    std::string  server       = defaults::server;
    bool         sync         = defaults::sync;
    int          threads      = defaults::threads;
    milliseconds tolerance    = defaults::tolerance;
    minutes      utc_offset   = 0min;


    template<typename T>
    void
    load_or_init(const std::string& key,
                 T& variable)
    {
        auto val = wups::storage::load<T>(key);
        if (!val)
            wups::storage::store(key, variable);
        else
            variable = *val;
    }


    void
    load()
    {
        try {
            load_or_init(key::auto_tz,      auto_tz);
            load_or_init(key::msg_duration, msg_duration);
            load_or_init(key::notify,       notify);
            load_or_init(key::server,       server);
            load_or_init(key::sync,         sync);
            load_or_init(key::threads,      threads);
            load_or_init(key::tolerance,    tolerance);
            load_or_init(key::utc_offset,   utc_offset);
            // logging::printf("Loaded settings.");
        }
        catch (std::exception& e) {
            logging::printf("Error loading config: %s", e.what());
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
            logging::printf("Error reloading config: %s", e.what());
        }
    }


    void
    save()
    {
        try {
            wups::storage::save();
            // logging::printf("Saved settings");
        }
        catch (std::exception& e) {
            logging::printf("Error saving config: %s", e.what());
        }
    }


    void
    migrate_old_config()
    {
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
            logging::printf("Migrated old config: %s + %s -> %s.",
                            time_utils::to_string(h).c_str(),
                            time_utils::to_string(m).c_str(),
                            time_utils::tz_offset_to_string(utc_offset).c_str());
        }
    }


    void
    set_and_store_utc_offset(minutes offset)
    {
        /*
         * Normally, `utc_offset` is saved on the config storage by the
         * `timezone_offset_item`. This function is supposed to be called by other parts
         * of the code, so it needs to manually store and save the new `utc_offset`.
         */
        utc_offset = offset;
        try {
            wups::storage::store(key::utc_offset, utc_offset);
            wups::storage::save();
        }
        catch (std::exception& e) {
            logging::printf("Error storing utc_offset: %s", e.what());
        }
    }

} // namespace cfg
