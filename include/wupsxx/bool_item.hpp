// SPDX-License-Identifier: MIT

#ifndef WUPSXX_BOOL_ITEM_HPP
#define WUPSXX_BOOL_ITEM_HPP

#include <memory>

#include "item.hpp"

#include "var_watch.hpp"


namespace wups::config {

    class bool_item : public item {

        var_watch<bool> variable;
        const bool default_value;
        std::string true_str;
        std::string false_str;

    public:

        bool_item(const std::optional<std::string>& key,
                  const std::string& label,
                  bool& variable,
                  bool default_value,
                  const std::string& true_str = "true",
                  const std::string& false_str = "false");

        static
        std::unique_ptr<bool_item>
        create(const std::optional<std::string>& key,
               const std::string& label,
               bool& variable,
               bool default_value,
               const std::string& true_str = "true",
               const std::string& false_str = "false");

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
