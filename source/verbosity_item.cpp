/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 * Copyright (C) 2024  Nightkingale
 *
 * SPDX-License-Identifier: MIT
 */

#include <algorithm>            // clamp()
#include <array>
#include <string.h>             // BSD strlcpy()

#include <wupsxx/cafe_glyphs.h>

#include "verbosity_item.hpp"


using namespace wups::config;


namespace {
    const std::array value_str = {
        "quiet",
        "normal",
        "verbose"
    };

    const char*
    value_to_str(int v)
    {
        v = std::clamp<int>(v, 0, value_str.size());
        return value_str[v];
    }
}


verbosity_item::verbosity_item(const std::string& label,
                               int& variable,
                               int default_value) :
    int_item{label,
             variable,
             default_value,
             0, 2, 2}
{}


std::unique_ptr<verbosity_item>
verbosity_item::create(const std::string& label,
                       int& variable,
                       int default_value)
{
    return std::make_unique<verbosity_item>(label, variable, default_value);
}


void
verbosity_item::get_display(char* buf, std::size_t size)
    const
{
    ::strlcpy(buf, value_to_str(variable), size);
}


void
verbosity_item::get_focused_display(char* buf, std::size_t size)
    const
{
    const char* left = "";
    const char* right = "";

    switch (variable) {
    case 0:
        right = " " CAFE_GLYPH_BTN_RIGHT;
        break;
    case 1:
        left = CAFE_GLYPH_BTN_LEFT " ";
        right = " " CAFE_GLYPH_BTN_RIGHT;
        break;
    case 2:
        left = CAFE_GLYPH_BTN_LEFT " ";
        break;
    }
    std::snprintf(buf, size,
                  "%s%s%s",
                  left, value_to_str(variable), right);
}
