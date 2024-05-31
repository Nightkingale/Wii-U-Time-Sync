// SPDX-License-Identifier: MIT

#include <cstdio>
#include <exception>

#include "wupsxx/bool_item.hpp"

#include "logging.hpp"
#include "nintendo_glyphs.h"
#include "wupsxx/storage.hpp"


namespace wups::config {

    bool_item::bool_item(const std::optional<std::string>& key,
                         const std::string& label,
                         bool& variable,
                         bool default_value,
                         const std::string& true_str,
                         const std::string& false_str) :
        item{key, label},
        variable(variable),
        default_value{default_value},
        true_str{true_str},
        false_str{false_str}
    {}


    std::unique_ptr<bool_item>
    bool_item::create(const std::optional<std::string>& key,
                      const std::string& label,
                      bool& variable,
                      bool default_value,
                      const std::string& true_str,
                      const std::string& false_str)
    {
        return std::make_unique<bool_item>(key, label,
                                           variable, default_value,
                                           true_str, false_str);
    }


    int
    bool_item::get_display(char* buf, std::size_t size)
        const
    {
        std::snprintf(buf, size, "%s",
                      *variable ? true_str.c_str() : false_str.c_str());
        return 0;
    }


    int
    bool_item::get_selected_display(char* buf, std::size_t size)
        const
    {
        const char* str = *variable ? true_str.c_str() : false_str.c_str();

        std::snprintf(buf, size,
                      "%s %s %s",
                      NIN_GLYPH_BTN_DPAD_LEFT,
                      str,
                      NIN_GLYPH_BTN_DPAD_RIGHT);
        return 0;
    }


    void
    bool_item::restore()
    {
        variable = default_value;
        on_changed();
    }


    void
    bool_item::on_input(WUPSConfigSimplePadData input,
                        WUPS_CONFIG_SIMPLE_INPUT repeat)
    {
        item::on_input(input, repeat);

        // Allow toggling with A, left or right.
        auto mask = WUPS_CONFIG_BUTTON_A | WUPS_CONFIG_BUTTON_LEFT | WUPS_CONFIG_BUTTON_RIGHT;

        if (input.buttons_d & mask)
            variable = !*variable;

        on_changed();
    }


    void
    bool_item::on_changed()
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
            logging::printf("Error storing bool: %s", e.what());
        }
    }

} // namespace wups::config
