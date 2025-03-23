/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2025  Daniel K. O.
 * Copyright (C) 2024  Nightkingale
 *
 * SPDX-License-Identifier: MIT
 */

#include <algorithm>            // clamp()
#include <cmath>                // abs()
#include <cstdio>               // snprintf()
#include <string.h>             // BSD strlcpy()

#include <wupsxx/cafe_glyphs.h>

#include "time_zone_offset_item.hpp"

#include "time_utils.hpp"
#include "utils.hpp"


using namespace std::literals;


time_zone_offset_item::time_zone_offset_item(wups::option<std::chrono::minutes>& opt) :
    var_item{opt},
    editing{field_id::hours}
{}


std::unique_ptr<time_zone_offset_item>
time_zone_offset_item::create(wups::option<std::chrono::minutes>& opt)
{
    return std::make_unique<time_zone_offset_item>(opt);
}


void
time_zone_offset_item::get_display(char* buf, std::size_t size)
    const
{
    auto str = time_utils::tz_offset_to_string(variable);
    ::strlcpy(buf, str.c_str(), size);
}


void
time_zone_offset_item::get_focused_display(char* buf, std::size_t size)
    const
{
    const char* left_right = editing == field_id::hours
        ? CAFE_GLYPH_BTN_RIGHT
        : CAFE_GLYPH_BTN_LEFT;

    const char* up_down;
    if (variable >= 14h)
        up_down = CAFE_GLYPH_BTN_DOWN;
    else if (variable <= -12h)
        up_down = CAFE_GLYPH_BTN_UP;
    else
        up_down = CAFE_GLYPH_BTN_UP_DOWN;

    auto str = time_utils::tz_offset_to_string(variable);

    // insert [] around the correct field (before or after ':')
    auto colon_pos = str.find(":");
    if (colon_pos == std::string::npos)
        throw std::logic_error{"failed to convert time zone offset to string"};
    switch (editing) {
    case field_id::hours:
        str = "[" + str.substr(0, colon_pos) + "]" + str.substr(colon_pos);
        break;
    case field_id::minutes:
        str = str.substr(0, colon_pos + 1) + "[" + str.substr(colon_pos + 1) + "]";
        break;
    }

    std::snprintf(buf, size,
                  "%s %s %s",
                  left_right, str.c_str(), up_down);
}


wups::focus_status
time_zone_offset_item::on_input(const wups::simple_pad_data& input)
{

    if (input.buttons_d & WUPS_CONFIG_BUTTON_LEFT)
        if (editing == field_id::minutes)
            editing = field_id::hours;

    if (input.buttons_d & WUPS_CONFIG_BUTTON_RIGHT)
        if (editing == field_id::hours)
            editing = field_id::minutes;

    if (input.pressed_or_long_held(WUPS_CONFIG_BUTTON_UP))
        variable += editing == field_id::hours ? 1h : 1min;

    if (input.pressed_or_long_held(WUPS_CONFIG_BUTTON_DOWN))
        variable -= editing == field_id::hours ? 1h : 1min;

    variable = std::clamp<std::chrono::minutes>(variable, -12h, 14h);

    return var_item::on_input(input);
}
