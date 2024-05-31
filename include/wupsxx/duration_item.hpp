// SPDX-License-Identifier: MIT

#ifndef WUPSXX_DURATION_ITEM_HPP
#define WUPSXX_DURATION_ITEM_HPP

#include <memory>
#include <chrono>

#include "item.hpp"
#include "var_watch.hpp"


namespace wups::config {

    class duration_item : public item {

    protected:

        var_watch<std::chrono::milliseconds> variable;
        const std::chrono::milliseconds default_value;
        std::chrono::milliseconds min_value;
        std::chrono::milliseconds max_value;
        std::chrono::milliseconds fast_increment;
        std::chrono::milliseconds slow_increment;

    public:

        duration_item(const std::optional<std::string>& key,
                      const std::string& label,
                      std::chrono::milliseconds& variable, std::chrono::milliseconds default_value,
                      std::chrono::milliseconds min_value, std::chrono::milliseconds max_value,
                      std::chrono::milliseconds fast_increment = std::chrono::milliseconds(1000),
                      std::chrono::milliseconds slow_increment = std::chrono::milliseconds(1));

        static
        std::unique_ptr<duration_item>
        create(const std::optional<std::string>& key,
               const std::string& label,
               std::chrono::milliseconds& variable, std::chrono::milliseconds default_value,
               std::chrono::milliseconds min_value, std::chrono::milliseconds max_value,
               std::chrono::milliseconds fast_increment = std::chrono::milliseconds(1000),
               std::chrono::milliseconds slow_increment = std::chrono::milliseconds(1));

        virtual int get_display(char* buf, std::size_t size) const override;

        virtual int get_selected_display(char* buf, std::size_t size) const override;

        virtual void restore() override;

        virtual void on_input(WUPSConfigSimplePadData input,
                              WUPS_CONFIG_SIMPLE_INPUT repeat) override;

    private:

        void on_changed();

    };

} // namespace wups::config

#endif