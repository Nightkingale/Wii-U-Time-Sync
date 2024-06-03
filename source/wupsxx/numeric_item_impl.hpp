// SPDX-License-Identifier: MIT

#ifndef WUPSXX_NUMERIC_ITEM_IMPL_HPP
#define WUPSXX_NUMERIC_ITEM_IMPL_HPP

#include <algorithm>            // clamp()
#include <chrono>
#include <cstdio>               // snprintf()
#include <exception>
#include <string.h>             // BSD strlcpy()

#include "wupsxx/numeric_item.hpp"

#include "logging.hpp"
#include "nintendo_glyphs.h"
#include "wupsxx/storage.hpp"
#include "time_utils.hpp"


using std::to_string;
using time_utils::to_string;


namespace wups::config {

    template<typename T>
    numeric_item<T>::numeric_item(const std::optional<std::string>& key,
                                  const std::string& label,
                                  T& variable, T default_value,
                                  T min_value, T max_value,
                                  T fast_increment, T slow_increment) :
        item{key, label},
        variable(variable),
        default_value{default_value},
        min_value{min_value},
        max_value{max_value},
        fast_increment{fast_increment},
        slow_increment{slow_increment}
    {}


    template<typename T>
    std::unique_ptr<numeric_item<T>>
    numeric_item<T>::create(const std::optional<std::string>& key,
                            const std::string& label,
                            T& variable, T default_value,
                            T min_value, T max_value,
                            T fast_increment, T slow_increment)
    {
        return std::make_unique<numeric_item<T>>(key, label,
                                                 variable, default_value,
                                                 min_value, max_value,
                                                 fast_increment, slow_increment);
    }


    template<typename T>
    int
    numeric_item<T>::get_display(char* buf, std::size_t size)
        const
    {
        std::string str = to_string(*variable);
        ::strlcpy(buf, str.c_str(), size);
        return 0;
    }


    template<typename T>
    int
    numeric_item<T>::get_selected_display(char* buf, std::size_t size)
        const
    {
        const char* slow_left = "";
        const char* slow_right = "";
        const char* fast_left = "";
        const char* fast_right = "";
        if (*variable > min_value) {
            slow_left = NIN_GLYPH_BTN_DPAD_LEFT " ";
            fast_left = NIN_GLYPH_BTN_L;
        } if (*variable < max_value) {
            slow_right = " " NIN_GLYPH_BTN_DPAD_RIGHT;
            fast_right = NIN_GLYPH_BTN_R;
        }
        std::string str = to_string(*variable);
        std::snprintf(buf, size,
                      "%s%s" "%s" "%s%s",
                      fast_left,
                      slow_left,
                      str.c_str(),
                      slow_right,
                      fast_right);
        return 0;
    }


    template<typename T>
    void
    numeric_item<T>::restore()
    {
        variable = default_value;
        on_changed();
    }


    template<typename T>
    void
    numeric_item<T>::on_input(WUPSConfigSimplePadData input,
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


    template<typename T>
    void
    numeric_item<T>::on_changed()
    {
        if (!key)
            return;
        if (!variable.changed())
            return;

        try {
            storage::store(*key, *variable);
            variable.reset();
        }
        catch (std::exception& e) {
            logging::printf("Error storing %s: %s", key->c_str(), e.what());
        }
    }

} // namespace wups::config


#endif
