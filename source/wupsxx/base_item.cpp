// SPDX-License-Identifier: MIT

#include <cstdio>
#include <stdexcept>

#include "base_item.hpp"


namespace wups {

    base_item::base_item(const std::string& key,
                         const std::string& name) :
        key{key},
        name{name}
    {
        WUPSConfigCallbacks_t cb;

        cb.getCurrentValueDisplay = [](void* ctx, char* buf, int size) -> int
        {
            if (!ctx)
                return -1;
            auto item = reinterpret_cast<const base_item*>(ctx);
            return item->get_current_value_display(buf, size);
        };

        cb.getCurrentValueSelectedDisplay = [](void* ctx, char* buf, int size) -> int
        {
            if (!ctx)
                return -1;
            auto item = reinterpret_cast<const base_item*>(ctx);
            return item->get_current_value_selected_display(buf, size);
        };

        cb.onSelected = [](void* ctx, bool is_selected)
        {
            if (!ctx)
                return;
            auto item = reinterpret_cast<base_item*>(ctx);
            item->on_selected(is_selected);
        };

        cb.restoreDefault = [](void* ctx)
        {
            if (!ctx)
                return;
            auto item = reinterpret_cast<base_item*>(ctx);
            item->restore();
        };

        cb.isMovementAllowed = [](void* ctx) -> bool
        {
            if (!ctx)
                return true;
            auto item = reinterpret_cast<const base_item*>(ctx);
            return item->is_movement_allowed();
        };

        cb.callCallback = [](void* ctx) -> bool
        {
            if (!ctx)
                return false;
            auto item = reinterpret_cast<base_item*>(ctx);
            return item->callback();
        };

        cb.onButtonPressed = [](void* ctx, WUPSConfigButtons button)
        {
            if (!ctx)
                return;
            auto item = reinterpret_cast<base_item*>(ctx);
            item->on_button_pressed(button);
        };

        // Called when WUPS is destroying the object.
        cb.onDelete = [](void* ctx)
        {
            if (!ctx)
                return;
            auto item = reinterpret_cast<base_item*>(ctx);
            item->handle = 0;
            delete item;
        };


        if (WUPSConfigItem_Create(&handle, key.c_str(), name.c_str(), cb, this) < 0)
            throw std::runtime_error{"could not create config item"};

    }


    base_item::~base_item()
    {
        if (handle)
            WUPSConfigItem_Destroy(handle);
    }


    int
    base_item::get_current_value_display(char* buf,
                                         std::size_t size)
        const
    {
        std::snprintf(buf, size, "NOT IMPLEMENTED");
        return 0;
    }


    int
    base_item::get_current_value_selected_display(char* buf,
                                                  std::size_t size)
        const
    {
        return get_current_value_display(buf, size);
    }


    void
    base_item::on_selected(bool)
    {}


    void
    base_item::restore()
    {}


    bool
    base_item::is_movement_allowed()
        const
    {
        return true;
    }


    bool
    base_item::callback()
    {
        return false;
    }


    void
    base_item::on_button_pressed(WUPSConfigButtons buttons)
    {
        if (buttons & WUPS_CONFIG_BUTTON_X)
            restore();
    }


} // namespace wups
