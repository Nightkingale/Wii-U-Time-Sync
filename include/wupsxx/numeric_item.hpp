// SPDX-License-Identifier: MIT

#ifndef WUPSXX_NUMERIC_ITEM_HPP
#define WUPSXX_NUMERIC_ITEM_HPP

#include <memory>

#include "item.hpp"
#include "var_watch.hpp"


namespace wups::config {


    template<typename T>
    class numeric_item : public item {

    protected:

        var_watch<T> variable;
        const T default_value;
        T min_value;
        T max_value;
        T fast_increment;
        T slow_increment;

    public:

        numeric_item(const std::optional<std::string>& key,
                     const std::string& label,
                     T& variable, T default_value,
                     T min_value, T max_value,
                     T fast_increment = T{10},
                     T slow_increment = T{1});

        static
        std::unique_ptr<numeric_item>
        create(const std::optional<std::string>& key,
               const std::string& label,
               T& variable, T default_value,
               T min_value, T max_value,
               T fast_increment = T{10},
               T slow_increment = T{1});


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
