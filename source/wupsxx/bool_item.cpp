// SPDX-License-Identifier: MIT

#include <cstdio>
#include <stdexcept>

#include "bool_item.hpp"
#include "nintendo_glyphs.hpp"
#include "storage.hpp"


namespace wups {

    bool_item::bool_item(const std::string& key,
                         const std::string& name,
                         bool& variable) :
        base_item{key, name},
        variable(variable),
        default_value{variable}
    {}


    int
    bool_item::get_current_value_display(char* buf, std::size_t size)
        const
    {
        std::snprintf(buf, size, "%s",
                      variable ? true_str.c_str() : false_str.c_str());
        return 0;
    }


    int
    bool_item::get_current_value_selected_display(char* buf, std::size_t size)
        const
    {
        if (variable)
            std::snprintf(buf, size, "%s %s  ", NIN_GLYPH_BTN_DPAD_LEFT, true_str.c_str());
        else
            std::snprintf(buf, size, "  %s %s", false_str.c_str(), NIN_GLYPH_BTN_DPAD_RIGHT);
        return 0;
    }


    void
    bool_item::restore()
    {
        variable = default_value;
    }


    bool
    bool_item::callback()
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
    bool_item::on_button_pressed(WUPSConfigButtons buttons)
    {
        base_item::on_button_pressed(buttons);

        if (buttons & WUPS_CONFIG_BUTTON_A)
            variable = !variable;

        if (buttons & WUPS_CONFIG_BUTTON_LEFT)
            variable = false;

        if (buttons & WUPS_CONFIG_BUTTON_RIGHT)
            variable = true;
    }

} // namespace wups
