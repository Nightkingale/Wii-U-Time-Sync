// SPDX-License-Identifier: MIT

#ifndef WUPSXX_TEXT_ITEM_HPP
#define WUPSXX_TEXT_ITEM_HPP

#include <memory>

#include "item.hpp"


namespace wups::config {

    // Note: this class doesn't do much on its own, so it's all public.

    struct text_item : item {

        std::string text;
        std::size_t max_width;
        std::size_t first = 0; // first visible character

        text_item(const std::optional<std::string>& key,
                  const std::string& label,
                  const std::string& text = "",
                  std::size_t max_width = 50);

        static
        std::unique_ptr<text_item>
        create(const std::optional<std::string>& key,
               const std::string& label,
               const std::string& text = "",
               std::size_t max_width = 50);


        virtual int get_display(char* buf, std::size_t size) const override;

        virtual int get_selected_display(char* buf, std::size_t size) const override;

        virtual void on_selected(bool is_selected) override;

        virtual void on_input(WUPSConfigSimplePadData input,
                              WUPS_CONFIG_SIMPLE_INPUT repeat) override;

    };

} // namespace wups::config

#endif
