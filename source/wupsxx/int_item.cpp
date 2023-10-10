// SPDX-License-Identifier: MIT

#include <algorithm>            // clamp()
#include <cstdio>
#include <stdexcept>

#include "int_item.hpp"
#include "storage.hpp"


namespace wups {


    int_item::int_item(const std::string& key,
                       const std::string& name,
                       int& variable,
                       int min_value,
                       int max_value) :
        base_item{key, name},
        variable(variable),
        default_value{variable},
        min_value{min_value},
        max_value{max_value}
    {}


    int
    int_item::get_current_value_display(char* buf, std::size_t size)
        const
    {
        std::snprintf(buf, size, "%d", variable);
        return 0;
    }


    int
    int_item::get_current_value_selected_display(char* buf, std::size_t size)
        const
    {
        char left = ' ';
        char right = ' ';
        if (variable > min_value)
            left = '<';
        if (variable < max_value)
            right = '>';
        std::snprintf(buf, size, "%c %d %c", left, variable, right);
        return 0;
    }


    void
    int_item::restore()
    {
        variable = default_value;
    }


    bool
    int_item::callback()
    {
        if (key.empty())
            return false;

        try {
            store(key, variable);
            return true;
        }
        catch (...) {
            return false;
        }
    }


    void
    int_item::on_button_pressed(WUPSConfigButtons buttons)
    {
        base_item::on_button_pressed(buttons);

        if (buttons & WUPS_CONFIG_BUTTON_LEFT)
            --variable;

        if (buttons & WUPS_CONFIG_BUTTON_RIGHT)
            ++variable;

        if (buttons & WUPS_CONFIG_BUTTON_L)
            variable -= 50;

        if (buttons & WUPS_CONFIG_BUTTON_R)
            variable += 50;

        variable = std::clamp(variable, min_value, max_value);
    }

} // namespace wups
