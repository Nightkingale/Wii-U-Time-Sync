// SPDX-License-Identifier: MIT

#ifndef TIMEZONE_QUERY_ITEM_HPP
#define TIMEZONE_QUERY_ITEM_HPP

#include <memory>

#include "wupsxx/item.hpp"
#include "wupsxx/var_watch.hpp"


class timezone_query_item : public wups::config::item {

    // We store the geolocation option as an integer, no point in parsing any complex
    // string since we need specific implementations for each service.

    wups::config::var_watch<int> variable;
    const int default_value;
    std::string text;

public:

    timezone_query_item(const std::string& key,
                        const std::string& label,
                        int& variable,
                        int default_value);

    static
    std::unique_ptr<timezone_query_item> create(const std::string& key,
                                                const std::string& label,
                                                int& variable,
                                                int default_value);


    int get_display(char* buf, std::size_t size) const override;

    int get_selected_display(char* buf, std::size_t size) const override;

    void restore() override;

    void on_input(WUPSConfigSimplePadData input,
                  WUPS_CONFIG_SIMPLE_INPUT repeat) override;

private:

    void on_changed();

    void run();

};

#endif
