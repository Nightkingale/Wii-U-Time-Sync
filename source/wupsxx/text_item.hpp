// SPDX-License-Identifier: MIT

#ifndef WUPSXX_TEXT_ITEM_HPP
#define WUPSXX_TEXT_ITEM_HPP

#include "base_item.hpp"

namespace wups {

    struct text_item : base_item {

        std::string text;

        text_item(const std::string& key = "",
                  const std::string& name = "",
                  const std::string& text = "");

        virtual int get_current_value_display(char* buf, std::size_t size) const override;

    };

} // namespace wups


#endif
