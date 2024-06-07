// SPDX-License-Identifier: MIT

#include <chrono>
#include <memory>               // make_unique()

#include "config_screen.hpp"

#include "cfg.hpp"
#include "timezone_offset_item.hpp"
#include "timezone_query_item.hpp"
#include "verbosity_item.hpp"
#include "wupsxx/bool_item.hpp"
#include "wupsxx/duration_items.hpp"
#include "wupsxx/int_item.hpp"
#include "wupsxx/text_item.hpp"
#include "wupsxx/numeric_item.hpp"


using wups::config::bool_item;
using wups::config::int_item;
using wups::config::milliseconds_item;
using wups::config::numeric_item;
using wups::config::seconds_item;
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
                              "on", "off"));

    cat.add(verbosity_item::create(cfg::key::notify,
                                   cfg::label::notify,
                                   cfg::notify,
                                   cfg::defaults::notify));

    cat.add(timezone_offset_item::create(cfg::key::utc_offset,
                                         cfg::label::utc_offset,
                                         cfg::utc_offset));

    cat.add(timezone_query_item::create(cfg::key::tz_service,
                                        cfg::label::tz_service,
                                        cfg::tz_service,
                                        cfg::defaults::tz_service));

    cat.add(bool_item::create(cfg::key::auto_tz,
                              cfg::label::auto_tz,
                              cfg::auto_tz,
                              cfg::defaults::auto_tz,
                              "on", "off"));

    cat.add(seconds_item::create(cfg::key::msg_duration,
                                 cfg::label::msg_duration,
                                 cfg::msg_duration,
                                 cfg::defaults::msg_duration,
                                 1s, 30s, 5s));

    cat.add(seconds_item::create(cfg::key::timeout,
                                  cfg::label::timeout,
                                  cfg::timeout,
                                  cfg::defaults::timeout,
                                  1s, 10s, 5s));

    cat.add(milliseconds_item::create(cfg::key::tolerance,
                                      cfg::label::tolerance,
                                      cfg::tolerance,
                                      cfg::defaults::tolerance,
                                      0ms, 5000ms, 100ms));

    cat.add(int_item::create(cfg::key::threads,
                             cfg::label::threads,
                             cfg::threads,
                             cfg::defaults::threads,
                             0, 8, 2));

    // show current NTP server address, no way to change it.
    cat.add(text_item::create(cfg::key::server,
                              cfg::label::server,
                              cfg::server));

    return cat;
}
