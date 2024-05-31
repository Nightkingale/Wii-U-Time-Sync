// SPDX-License-Identifier: MIT

#include <array>
#include <chrono>
#include <cstdio>               // snprintf()

#include "wupsxx/item.hpp"

#include "wupsxx/config_error.hpp"


using namespace std::literals;


namespace wups::config {

    namespace dispatchers {

        int32_t
        get_display(void* ctx, char* buf, int32_t size)
        {
            auto i = static_cast<const item*>(ctx);
            return i->get_display(buf, size);
        }


        int32_t
        get_selected_display(void* ctx, char* buf, int32_t size)
        {
            auto i = static_cast<const item*>(ctx);
            return i->get_selected_display(buf, size);
        }


        bool
        is_movement_allowed(void* ctx)
        {
            auto i = static_cast<const item*>(ctx);
            return i->is_movement_allowed();
        }


        void
        on_close(void* ctx)
        {
            auto i = static_cast<item*>(ctx);
            i->on_close();
        }


        void
        on_delete(void* ctx)
        {
            auto i = static_cast<item*>(ctx);
            i->release(); // don't destroy the handle, it's already happening
            delete i;
        }


        void
        on_input(void* ctx, WUPSConfigSimplePadData input)
        {
            // Here we implement a "repeat" function.
            using clock = std::chrono::steady_clock;
            using time_point = clock::time_point;

            constexpr auto repeat_delay = 500ms;
            static std::array<time_point, 16> pressed_time{};
            auto now = clock::now();

            unsigned repeat = 0;
            for (unsigned b = 0; b < 16; ++b) {
                unsigned mask = 1u << b;
                if (input.buttons_d & mask)
                    pressed_time[b] = now;

                if (input.buttons_h & mask)
                    // if button was held long enough, flag it as being on a repeat state
                    if (now - pressed_time[b] >= repeat_delay)
                        repeat |= mask;

                if (input.buttons_r & mask)
                    pressed_time[b] = {};
            }

            auto i = static_cast<item*>(ctx);
            i->on_input(input, static_cast<WUPS_CONFIG_SIMPLE_INPUT>(repeat));
        }


        void
        on_input_ex(void* ctx, WUPSConfigComplexPadData input)
        {
            // TODO: implement "repeat" functionality for extended input too
            auto i = static_cast<item*>(ctx);
            i->on_input(input);
        }


        void
        on_selected(void* ctx, bool is_selected)
        {
            auto i = static_cast<item*>(ctx);
            i->on_selected(is_selected);
        }


        void
        restore_default(void* ctx)
        {
            auto i = static_cast<item*>(ctx);
            i->restore();
        }

    } // namespace dispatchers


    item::item(const std::optional<std::string>& key,
               const std::string& label) :
        key{key}
    {
        WUPSConfigAPIItemOptionsV2 options {
            .displayName = label.c_str(),
            .context = this,
            .callbacks = {
                // Note: do not sort, must be initialized in the order of declaration.
                .getCurrentValueDisplay         = dispatchers::get_display,
                .getCurrentValueSelectedDisplay = dispatchers::get_selected_display,
                .onSelected                     = dispatchers::on_selected,
                .restoreDefault                 = dispatchers::restore_default,
                .isMovementAllowed              = dispatchers::is_movement_allowed,
                .onCloseCallback                = dispatchers::on_close,
                .onInput                        = dispatchers::on_input,
                .onInputEx                      = dispatchers::on_input_ex,
                .onDelete                       = dispatchers::on_delete,
            }
        };

        auto status = WUPSConfigAPI_Item_Create(options, &handle);
        if (status != WUPSCONFIG_API_RESULT_SUCCESS)
            throw config_error{status, "could not create config item \"" + label + "\""};
    }


    item::~item()
    {
        if (handle.handle)
            WUPSConfigAPI_Item_Destroy(handle);
    }


    void
    item::release()
    {
        handle = {};
    }


    int
    item::get_display(char* buf,
                      std::size_t size)
        const
    {
        std::snprintf(buf, size, "NOT IMPLEMENTED");
        return 0;
    }


    int
    item::get_selected_display(char* buf,
                               std::size_t size)
        const
    {
        return get_display(buf, size);
    }


    void
    item::on_selected(bool)
    {}


    void
    item::restore()
    {}


    bool
    item::is_movement_allowed()
        const
    {
        return true;
    }


    void
    item::on_close()
    {}


    void
    item::on_input(WUPSConfigSimplePadData input,
                   WUPS_CONFIG_SIMPLE_INPUT /*repeat*/)
    {
        if (input.buttons_d & WUPS_CONFIG_BUTTON_X)
            restore();
    }


    void
    item::on_input(WUPSConfigComplexPadData /*input*/)
    {}

} // namespace wups::config
