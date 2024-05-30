// SPDX-License-Identifier: MIT

#ifndef TIMEZONE_QUERY_ITEM_HPP
#define TIMEZONE_QUERY_ITEM_HPP

#include <memory>

#include "wupsxx/text_item.hpp"


struct timezone_query_item : wups::config::text_item {

    timezone_query_item();

    static
    std::unique_ptr<timezone_query_item> create();

    void on_input(WUPSConfigSimplePadData input,
                  WUPS_CONFIG_SIMPLE_INPUT repeat) override;

    void run();

};

#endif
