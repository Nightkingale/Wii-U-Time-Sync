// SPDX-License-Identifier: MIT

#include <algorithm>            // clamp()

#include "verbosity_item.hpp"

#include "nintendo_glyphs.h"


namespace {
    const char* value_str[] = {
        "no",
        "yes",
        "verbose"
    };

    const char*
    value_to_str(int v)
    {
        v = std::clamp(v, 0, 2);
        return value_str[v];
    }
}


verbosity_item::verbosity_item(const std::string& key,
                               const std::string& label,
                               int& variable,
                               int default_value) :
    int_item{key, label,
             variable, default_value,
             0, 2, 2}
{}


std::unique_ptr<verbosity_item>
verbosity_item::create(const std::string& key,
                       const std::string& label,
                       int& variable,
                       int default_value)
{
    return std::make_unique<verbosity_item>(key, label, variable, default_value);
}


int
verbosity_item::get_display(char* buf, std::size_t size)
    const
{
    std::snprintf(buf, size, "%s", value_to_str(*variable));
    return 0;
}


int
verbosity_item::get_selected_display(char* buf, std::size_t size)
    const
{
    const char* left = "";
    const char* right = "";

    switch (*variable) {
    case 0:
        right = " " NIN_GLYPH_BTN_DPAD_RIGHT;
        break;
    case 1:
        left = NIN_GLYPH_BTN_DPAD_LEFT " ";
        right = " " NIN_GLYPH_BTN_DPAD_RIGHT;
        break;
    case 2:
        left = NIN_GLYPH_BTN_DPAD_LEFT " ";
        break;
    }
    std::snprintf(buf, size, "%s%s%s", left, value_to_str(*variable), right);
    return 0;
}
