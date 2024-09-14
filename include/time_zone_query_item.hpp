/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TIME_ZONE_QUERY_ITEM_HPP
#define TIME_ZONE_QUERY_ITEM_HPP

#include <memory>

#include <wupsxx/var_item.hpp>


class time_zone_query_item : public wups::config::var_item<int> {

    // We store the geolocation option as an integer, no point in parsing any complex
    // string since we need specific implementations for each service.

    std::string text;

public:

    time_zone_query_item(const std::string& label,
                         int& variable,
                         int default_value);

    static
    std::unique_ptr<time_zone_query_item>
    create(const std::string& label,
           int& variable,
           int default_value);


    virtual
    void
    get_display(char* buf, std::size_t size) const override;

    virtual
    void
    get_focused_display(char* buf, std::size_t size) const override;

    virtual
    wups::config::focus_status
    on_input(const wups::config::simple_pad_data& input) override;

private:

    void run();

};

#endif
