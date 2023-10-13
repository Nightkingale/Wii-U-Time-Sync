// SPDX-License-Identifier: MIT

#ifndef WUPSXX_BASE_ITEM_HPP
#define WUPSXX_BASE_ITEM_HPP

#include <string>

#include <wups.h>


namespace wups {

    struct base_item {

        WUPSConfigItemHandle handle = 0;
        std::string key;
        std::string name;


        base_item(const std::string& key,
                  const std::string& name);

        // disallow moving, since the callbacks store the `this` pointer.
        base_item(base_item&&) = delete;


        virtual ~base_item();


        virtual int get_current_value_display(char* buf, std::size_t size) const;

        virtual int get_current_value_selected_display(char* buf, std::size_t size) const;

        virtual void on_selected(bool is_selected);

        virtual void restore();

        virtual bool is_movement_allowed() const;

        virtual bool callback();

        virtual void on_button_pressed(WUPSConfigButtons buttons);

    };

} // namespace wups

#endif
