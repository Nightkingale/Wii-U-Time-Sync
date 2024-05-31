// SPDX-License-Identifier: MIT

#ifndef VERBOSITY_ITEM_HPP
#define VERBOSITY_ITEM_HPP

#include <memory>

#include "wupsxx/int_item.hpp"


struct verbosity_item : wups::config::int_item {

    verbosity_item(const std::string& key,
                   const std::string& label,
                   int& variable,
                   int default_value);

    static
    std::unique_ptr<verbosity_item>
    create(const std::string& notify_key,
           const std::string& label,
           int& variable,
           int default_value);


    virtual int get_display(char* buf, std::size_t size) const override;

    virtual int get_selected_display(char* buf, std::size_t size) const override;

};


#endif
