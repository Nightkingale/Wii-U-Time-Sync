// SPDX-License-Identifier: MIT

#ifndef WUPSXX_INT_ITEM_HPP
#define WUPSXX_INT_ITEM_HPP

#include <memory>

#include "item.hpp"
#include "var_watch.hpp"


namespace wups::config {

    class int_item : public item {

    protected:

        var_watch<int> variable;
        const int default_value;
        int min_value;
        int max_value;
        int fast_increment;
        int slow_increment;

    public:

        int_item(const std::optional<std::string>& key,
                 const std::string& label,
                 int& variable, int default_value,
                 int min_value, int max_value,
                 int fast_increment = 10,
                 int slow_increment = 1);

        static
        std::unique_ptr<int_item>
        create(const std::optional<std::string>& key,
               const std::string& label,
               int& variable, int default_value,
               int min_value, int max_value,
               int fast_increment = 10,
               int slow_increment = 1);

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
