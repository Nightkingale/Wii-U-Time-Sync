// SPDX-License-Identifier: MIT

#include "cfg.hpp"

#include "utc.hpp"
#include "wupsxx/storage.hpp"


namespace cfg {

    namespace key {
        const char* hours        = "hours";
        const char* minutes      = "minutes";
        const char* msg_duration = "msg_duration";
        const char* notify       = "notify";
        const char* server       = "server";
        const char* sync         = "sync";
        const char* tolerance    = "tolerance";
    }


    int         hours        = 0;
    int         minutes      = 0;
    int         msg_duration = 5;
    bool        notify       = false;
    std::string server       = "pool.ntp.org";
    bool        sync         = false;
    int         tolerance    = 250;


    template<typename T>
    void
    load_or_init(const std::string& key,
                 T& variable)
    {
        auto val = wups::load<T>(key);
        if (!val)
            wups::store(key, variable);
        else
            variable = *val;
    }


    void
    load()
    {
        load_or_init(key::hours,        hours);
        load_or_init(key::minutes,      minutes);
        load_or_init(key::msg_duration, msg_duration);
        load_or_init(key::notify,       notify);
        load_or_init(key::server,       server);
        load_or_init(key::sync,         sync);
        load_or_init(key::tolerance,    tolerance);
    }


    void
    update_utc_offset()
    {
        double offset_seconds = (hours * 60.0 + minutes) * 60.0;
        utc::timezone_offset = offset_seconds;
    }

} // namespace cfg
