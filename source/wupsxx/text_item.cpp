// SPDX-License-Identifier: MIT

#include <algorithm>
#include <cstdio>
#include <stdexcept>

#include "text_item.hpp"


using namespace std::literals;

namespace wups {

    text_item::text_item(const std::string& key,
                         const std::string& name,
                         const std::string& text) :
        base_item{key, name},
        text{text}
    {}


    int
    text_item::get_current_value_display(char* buf,
                                         std::size_t size)
        const
    {
        auto width = std::min<int>(size - 1, max_width);

        std::snprintf(buf, size,
                      "%*.*s",
                      width,
                      width,
                      text.c_str() + start);

        return 0;
    }


    void
    text_item::on_selected(bool is_selected)
    {
        if (!is_selected)
            start = 0;
    }


    void
    text_item::on_button_pressed(WUPSConfigButtons buttons)
    {
        base_item::on_button_pressed(buttons);

        if (text.empty())
            return;

        int tsize = static_cast<int>(text.size());

        if (tsize <= max_width)
            return;

        if (buttons & WUPS_CONFIG_BUTTON_LEFT)
            start -= 5;

        if (buttons & WUPS_CONFIG_BUTTON_RIGHT)
            start += 5;

        if (start >= tsize - max_width)
            start = tsize - max_width;
        if (start < 0)
            start = 0;
    }

} // namespace wups
