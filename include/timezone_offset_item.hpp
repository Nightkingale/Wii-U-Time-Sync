// SPDX-License-Identifier: MIT

#ifndef TIMEZONE_OFFSET_ITEM_HPP
#define TIMEZONE_OFFSET_ITEM_HPP

#include <chrono>
#include <memory>

#include "wupsxx/item.hpp"
#include "wupsxx/var_watch.hpp"


struct timezone_offset_item : wups::config::item {

    wups::config::var_watch<std::chrono::minutes> variable;

    timezone_offset_item(const std::string& key,
                         const std::string& label,
                         std::chrono::minutes& variable);

    static
    std::unique_ptr<timezone_offset_item>
    create(const std::string& key,
           const std::string& label,
           std::chrono::minutes& variable);

    virtual int get_display(char* buf, std::size_t size) const override;

    virtual int get_selected_display(char* buf, std::size_t size) const override;

    virtual void restore() override;

    virtual void on_input(WUPSConfigSimplePadData input,
                          WUPS_CONFIG_SIMPLE_INPUT repeat) override;

private:

    void on_changed();

};


#endif
