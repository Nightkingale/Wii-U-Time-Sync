// SPDX-License-Identifier: MIT

#ifndef WUPSXX_INT_ITEM_HPP
#define WUPSXX_INT_ITEM_HPP

#include "base_item.hpp"


namespace wups {

    struct int_item : base_item {

        int& variable;
        int default_value = 0;
        int min_value;
        int max_value;


        int_item(const std::string& key,
                 const std::string& name,
                 int& variable,
                 int min_value,
                 int max_value);


        virtual int get_current_value_display(char* buf, std::size_t size) const override;

        virtual int get_current_value_selected_display(char* buf, std::size_t size) const override;

        virtual void restore() override;

        virtual bool callback() override;

        virtual void on_button_pressed(WUPSConfigButtons buttons) override;

    };

} // namespace wups

#endif
