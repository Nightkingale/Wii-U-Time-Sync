/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TIME_ZONE_OFFSET_ITEM_HPP
#define TIME_ZONE_OFFSET_ITEM_HPP

#include <chrono>
#include <memory>

#include <wupsxx/var_item.hpp>


struct time_zone_offset_item : wups::var_item<std::chrono::minutes> {

    enum field_id : unsigned {
        hours,
        minutes,
    };


    field_id editing;


    time_zone_offset_item(wups::option<std::chrono::minutes>& opt);

    static
    std::unique_ptr<time_zone_offset_item>
    create(wups::option<std::chrono::minutes>& opt);

    virtual
    void
    get_display(char* buf,
                std::size_t size)
        const override;

    virtual
    void
    get_focused_display(char* buf,
                        std::size_t size)
        const override;

    virtual
    wups::focus_status
    on_input(const wups::simple_pad_data& input)
        override;

private:

    void
    on_changed();

};

#endif
