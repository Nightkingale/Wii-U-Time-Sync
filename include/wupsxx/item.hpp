// SPDX-License-Identifier: MIT

#ifndef WUPSXX_ITEM_HPP
#define WUPSXX_ITEM_HPP

#include <cstddef>              // size_t
#include <optional>
#include <string>

#include <wups.h>


namespace wups::config {

    class item {

        WUPSConfigItemHandle handle;

    protected:

        std::optional<std::string> key;

    public:

        item(const std::optional<std::string>& key,
             const std::string& label);

        // Disallow moving, since the callbacks store the `this` pointer.
        item(item&&) = delete;

        virtual ~item();

        // Gives up ownership of the handle.
        void release();


        virtual int get_display(char* buf, std::size_t size) const;

        virtual int get_selected_display(char* buf, std::size_t size) const;

        virtual void on_selected(bool is_selected);

        virtual void restore();

        virtual bool is_movement_allowed() const;

        virtual void on_close();

        virtual void on_input(WUPSConfigSimplePadData input,
                              WUPS_CONFIG_SIMPLE_INPUT repeat);

        virtual void on_input(WUPSConfigComplexPadData input);


        friend class category;

    };

} // namespace wups::config

#endif
