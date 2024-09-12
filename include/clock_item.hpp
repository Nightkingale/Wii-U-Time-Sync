/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef CLOCK_ITEM_HPP
#define CLOCK_ITEM_HPP

#include <map>
#include <memory>               // unique_ptr<>
#include <string>

#include <wupsxx/button_item.hpp>
#include <wupsxx/text_item.hpp>


struct clock_item : wups::config::button_item {

    struct server_info {
        wups::config::text_item* name = nullptr;
        wups::config::text_item* correction = nullptr;
        wups::config::text_item* latency = nullptr;
    };


    std::string now_str;
    std::string diff_str;
    std::map<std::string, server_info> server_infos;


    clock_item();

    static
    std::unique_ptr<clock_item>
    create();


    virtual
    void
    on_started() override;


    void
    update_status_msg();


    void
    run();

};

#endif
