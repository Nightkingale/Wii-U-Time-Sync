// SPDX-License-Identifier: MIT

#ifndef WUPSXX_TEXT_ITEM_HPP
#define WUPSXX_TEXT_ITEM_HPP

#include "base_item.hpp"

namespace wups {

    struct text_item : base_item {

        std::string text;
        int max_width = 50;
        int start = 0;

        text_item(const std::string& key = "",
                  const std::string& name = "",
                  const std::string& text = "");

        virtual int get_current_value_display(char* buf, std::size_t size) const override;

        virtual void on_selected(bool is_selected) override;

        virtual void on_button_pressed(WUPSConfigButtons buttons) override;
    };

} // namespace wups


#endif
