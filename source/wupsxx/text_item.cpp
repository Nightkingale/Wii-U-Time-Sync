// SPDX-License-Identifier: MIT

#include <algorithm>            // clamp(), min()
#include <cstdio>               // snprintf()

#include "wupsxx/text_item.hpp"

#include "nintendo_glyphs.h"


namespace {

    const std::string left_glyph = NIN_GLYPH_BTN_DPAD_LEFT " ";
    const std::string right_glyph = " " NIN_GLYPH_BTN_DPAD_RIGHT;

}


namespace wups::config {

    text_item::text_item(const std::optional<std::string>& key,
                         const std::string& label,
                         const std::string& text,
                         std::size_t max_width) :
        item{key, label},
        text{text},
        max_width{std::min<std::size_t>(max_width, 79)}
    {}


    std::unique_ptr<text_item>
    text_item::create(const std::optional<std::string>& key,
                      const std::string& label,
                      const std::string& text,
                      std::size_t max_width)
    {
        return std::make_unique<text_item>(key, label, text, max_width);
    }


    int
    text_item::get_display(char* buf,
                           std::size_t size)
        const
    {
        // Note: `buf` is a C string, it needs a null terminator at the end,
        // so the effective `width` is one less than `size`.
        std::size_t width = std::min(size - 1, max_width);

        if (width >= text.size()) {
            // Easy case: text fits, just show it all.
            std::snprintf(buf, size, "%s", text.c_str());
            return 0;
        }

        const char* ellipsis = "…";

        std::string prefix;
        if (first > 0)
            prefix = ellipsis;
        if (width < prefix.size()) // sanity check
            return -1;
        width -= prefix.size();

        std::size_t last = first + width;
        std::string suffix;
        if (last < text.size())
            suffix = ellipsis;
        if (width < suffix.size()) // sanity check
            return -1;
        width -= suffix.size();

        std::string slice = text.substr(first, width);

        std::snprintf(buf, size,
                      "%s%s%s",
                      prefix.c_str(),
                      slice.c_str(),
                      suffix.c_str());

        return 0;
    }


    int
    text_item::get_selected_display(char* buf,
                                    std::size_t size)
        const
    {
        // Note: `buf` is a C string, it needs a null terminator at the end,
        // so the effective `width` is one less than `size`.
        std::size_t width = std::min(size - 1, max_width);

        if (width >= text.size()) {
            // Easy case: text fits, just show it all.
            std::snprintf(buf, size, "%s", text.c_str());
            return 0;
        }

        std::string prefix;
        if (first > 0)
            prefix = left_glyph;
        if (width < prefix.size()) // sanity check
            return -1;
        width -= prefix.size();

        std::size_t last = first + width;
        std::string suffix;
        if (last < text.size())
            suffix = right_glyph;
        if (width < suffix.size()) // sanity check
            return -1;
        width -= suffix.size();

        std::string slice = text.substr(first, width);

        std::snprintf(buf, size,
                      "%s%s%s",
                      prefix.c_str(),
                      slice.c_str(),
                      suffix.c_str());

        return 0;
    }


    void
    text_item::on_selected(bool /*is_selected*/)
    {
        // if (!is_selected)
        //     first = 0;
    }


    void
    text_item::on_input(WUPSConfigSimplePadData input,
                        WUPS_CONFIG_SIMPLE_INPUT repeat)
    {
        item::on_input(input, repeat);

        if (text.empty())
            return;

        // If text is fully visible, no scrolling happens.
        if (text.size() <= max_width)
            return;

        // Handle text scrolling

        const std::size_t max_first = text.size() - max_width + left_glyph.size();

        if (input.buttons_d & WUPS_CONFIG_BUTTON_LEFT ||
            repeat & WUPS_CONFIG_BUTTON_LEFT)
            if (first > 0) {
                --first;
            }

        if (input.buttons_d & WUPS_CONFIG_BUTTON_RIGHT ||
            repeat & WUPS_CONFIG_BUTTON_RIGHT)
            if (first < max_first) {
                ++first;
            }

        if (input.buttons_d & WUPS_CONFIG_BUTTON_L)
            first = 0;

        if (input.buttons_d & WUPS_CONFIG_BUTTON_R)
            first = max_first;
    }

} // namespace wups::config
