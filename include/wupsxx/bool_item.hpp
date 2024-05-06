// SPDX-License-Identifier: MIT

#ifndef WUPSXX_BOOL_ITEM_HPP
#define WUPSXX_BOOL_ITEM_HPP

#include <string>

#include "base_item.hpp"

namespace wups {

    struct bool_item : base_item {

        bool& variable;
        bool default_value;
        std::string true_str = "true";
        std::string false_str = "false";


        bool_item(const std::string& key,
                  const std::string& name,
                  bool& variable);


        virtual int get_current_value_display(char* buf, std::size_t size) const override;

        virtual int get_current_value_selected_display(char* buf, std::size_t size) const override;

        virtual void restore() override;

        virtual bool callback() override;

        virtual void on_button_pressed(WUPSConfigButtons button) override;

    };

} // namespace wups

#endif
