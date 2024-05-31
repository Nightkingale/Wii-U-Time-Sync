// SPDX-License-Identifier: MIT

#include <chrono>
#include <memory>               // make_unique()

#include "wupsxx/bool_item.hpp"
#include "wupsxx/int_item.hpp"
#include "wupsxx/text_item.hpp"

#include "config_screen.hpp"

#include "cfg.hpp"
#include "timezone_offset_item.hpp"
#include "timezone_query_item.hpp"
#include "verbosity_item.hpp"


using wups::config::bool_item;
using wups::config::int_item;
using wups::config::text_item;

using namespace std::literals;


wups::config::category
make_config_screen()
{
    wups::config::category cat{"Configuration"};

    cat.add(bool_item::create(cfg::key::sync,
                              cfg::label::sync,
                              cfg::sync,
                              cfg::defaults::sync,
                              "yes", "no"));

    cat.add(verbosity_item::create(cfg::key::notify,
                                   cfg::label::notify,
                                   cfg::notify,
                                   cfg::defaults::notify));

    cat.add(int_item::create(cfg::key::msg_duration,
                             cfg::label::msg_duration,
                             cfg::msg_duration,
                             cfg::defaults::msg_duration,
                             1, 30, 5));

    cat.add(timezone_offset_item::create(cfg::key::utc_offset,
                                         cfg::label::utc_offset,
                                         cfg::utc_offset));

    cat.add(timezone_query_item::create());

    cat.add(bool_item::create(cfg::key::auto_tz,
                              cfg::label::auto_tz,
                              cfg::auto_tz,
                              cfg::defaults::auto_tz,
                              "yes", "no"));

    cat.add(int_item::create(cfg::key::tolerance,
                             cfg::label::tolerance,
                             cfg::tolerance,
                             cfg::defaults::tolerance,
                             0, 5000, 100));

    // show current NTP server address, no way to change it.
    cat.add(text_item::create(cfg::key::server,
                              cfg::label::server,
                              cfg::server));

    cat.add(int_item::create(cfg::key::threads,
                             cfg::label::threads,
                             cfg::threads,
                             cfg::defaults::threads,
                             0, 8, 2));

    return cat;
}
