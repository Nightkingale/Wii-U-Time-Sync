// SPDX-License-Identifier: MIT

#include <algorithm>            // clamp()
#include <cstdio>
#include <exception>

#include "wupsxx/duration_item.hpp"

#include "logging.hpp"
#include "nintendo_glyphs.h"
#include "wupsxx/storage.hpp"


namespace wups::config {

    duration_item::duration_item(const std::optional<std::string>& key,
                                 const std::string& label,
                                 std::chrono::milliseconds& variable, std::chrono::milliseconds default_value,
                                 std::chrono::milliseconds min_value, std::chrono::milliseconds max_value,
                                 std::chrono::milliseconds fast_increment, std::chrono::milliseconds slow_increment) :
        item{key, label},
        variable(variable),
        default_value{default_value},
        min_value{min_value},
        max_value{max_value},
        fast_increment{fast_increment},
        slow_increment{slow_increment}
    {}


    std::unique_ptr<duration_item>
    duration_item::create(const std::optional<std::string>& key,
                          const std::string& label,
                          std::chrono::milliseconds& variable, std::chrono::milliseconds default_value,
                          std::chrono::milliseconds min_value, std::chrono::milliseconds max_value,
                          std::chrono::milliseconds fast_increment, std::chrono::milliseconds slow_increment)
    {
        return std::make_unique<duration_item>(key, label,
                                               variable, default_value,
                                               min_value, max_value,
                                               fast_increment, slow_increment);
    }


    int
    duration_item::get_display(char* buf, std::size_t size)
        const
    {
        std::snprintf(buf, size, "%lld", variable->count());
        return 0;
    }


    int
    duration_item::get_selected_display(char* buf, std::size_t size)
        const
    {
        const char* slow_left = "";
        const char* slow_right = "";
        const char* fast_left = "";
        const char* fast_right = "";
        if (variable > min_value) {
            slow_left = NIN_GLYPH_BTN_DPAD_LEFT " ";
            fast_left = NIN_GLYPH_BTN_L;
        } if (variable < max_value) {
            slow_right = " " NIN_GLYPH_BTN_DPAD_RIGHT;
            fast_right = NIN_GLYPH_BTN_R;
        }
        std::snprintf(buf, size,
                      "%s%s%lld%s%s",
                      fast_left,
                      slow_left,
                      variable->count(),
                      slow_right,
                      fast_right);
        return 0;
    }


    void
    duration_item::restore()
    {
        variable = default_value;
        on_changed();
    }


    void
    duration_item::on_input(WUPSConfigSimplePadData input,
                            WUPS_CONFIG_SIMPLE_INPUT repeat)
    {
        item::on_input(input, repeat);

        if (input.buttons_d & WUPS_CONFIG_BUTTON_LEFT ||
            repeat & WUPS_CONFIG_BUTTON_LEFT)
            variable -= slow_increment;

        if (input.buttons_d & WUPS_CONFIG_BUTTON_RIGHT ||
            repeat & WUPS_CONFIG_BUTTON_RIGHT)
            variable += slow_increment;

        if (input.buttons_d & WUPS_CONFIG_BUTTON_L ||
            repeat & WUPS_CONFIG_BUTTON_L)
            variable -= fast_increment;

        if (input.buttons_d & WUPS_CONFIG_BUTTON_R ||
            repeat & WUPS_CONFIG_BUTTON_R)
            variable += fast_increment;

        variable = std::clamp(*variable, min_value, max_value);

        on_changed();
    }


    void
    duration_item::on_changed()
    {
        if (!key)
            return;
        if (!variable.changed())
            return;

        try {
            storage::store(*key, variable->count());
            variable.reset();
        }
        catch (std::exception& e) {
            logging::printf("Error storing duration: %s", e.what());
        }
    }

} // namespace wups::config