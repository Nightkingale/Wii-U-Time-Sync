// SPDX-License-Identifier: MIT

#include <algorithm>            // clamp()
#include <cmath>                // abs()
#include <cstdio>               // snprintf()
#include <string.h>             // BSD strlcpy()

#include "timezone_offset_item.hpp"

#include "logging.hpp"
#include "nintendo_glyphs.h"
#include "utils.hpp"
#include "wupsxx/storage.hpp"


using namespace std::literals;


timezone_offset_item::timezone_offset_item(const std::string& key,
                                           const std::string& label,
                                           std::chrono::minutes& variable) :
    item{key, label},
    variable(variable)
{}


std::unique_ptr<timezone_offset_item>
timezone_offset_item::create(const std::string& key,
                             const std::string& label,
                             std::chrono::minutes& variable)
{
    return std::make_unique<timezone_offset_item>(key, label, variable);
}


int
timezone_offset_item::get_display(char* buf, std::size_t size)
    const
{
    auto str = utils::tz_offset_to_string(*variable);
    ::strlcpy(buf, str.c_str(), size);
    return 0;
}


int
timezone_offset_item::get_selected_display(char* buf, std::size_t size)
    const
{
    const char* slow_left = "";
    const char* slow_right = "";
    const char* fast_left = "";
    const char* fast_right = "";
    if (*variable > -12h) {
        slow_left = NIN_GLYPH_BTN_DPAD_LEFT;
        fast_left = NIN_GLYPH_BTN_L;
    } if (*variable < 14h) {
        slow_right = NIN_GLYPH_BTN_DPAD_RIGHT;
        fast_right = NIN_GLYPH_BTN_R;
    }

    auto str = utils::tz_offset_to_string(*variable);
    std::snprintf(buf, size, "%s%s %s %s%s",
                  fast_left, slow_left,
                  str.c_str(),
                  slow_right, fast_right);
    return 0;
}


void
timezone_offset_item::restore()
{
    variable = 0min;
    on_changed();
}


void
timezone_offset_item::on_input(WUPSConfigSimplePadData input,
                               WUPS_CONFIG_SIMPLE_INPUT repeat)
{
    item::on_input(input, repeat);

    if (input.buttons_d & WUPS_CONFIG_BUTTON_LEFT ||
        repeat & WUPS_CONFIG_BUTTON_LEFT)
        variable -= 1min;

    if (input.buttons_d & WUPS_CONFIG_BUTTON_RIGHT ||
        repeat & WUPS_CONFIG_BUTTON_RIGHT)
        variable += 1min;

    if (input.buttons_d & WUPS_CONFIG_BUTTON_L ||
        repeat & WUPS_CONFIG_BUTTON_L)
        variable -= 1h;

    if (input.buttons_d & WUPS_CONFIG_BUTTON_R ||
        repeat & WUPS_CONFIG_BUTTON_R)
        variable += 1h;

    variable = std::clamp<std::chrono::minutes>(*variable, -12h, 14h);

    on_changed();
}


void
timezone_offset_item::on_changed()
{
    if (!key)
        return;
    if (!variable.changed())
        return;

    try {
        wups::storage::store<int>(*key, variable->count());
        variable.reset();
    }
    catch (std::exception& e) {
        logging::printf("Error storing timezone offset: %s", e.what());
    }
}
