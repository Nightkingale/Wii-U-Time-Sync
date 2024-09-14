/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef VERBOSITY_ITEM_HPP
#define VERBOSITY_ITEM_HPP

#include <memory>

#include "wupsxx/int_item.hpp"


struct verbosity_item : wups::config::int_item {

    verbosity_item(const std::string& label,
                   int& variable,
                   int default_value);

    static
    std::unique_ptr<verbosity_item>
    create(const std::string& label,
           int& variable,
           int default_value);


    virtual void get_display(char* buf, std::size_t size) const override;

    virtual void get_focused_display(char* buf, std::size_t size) const override;

};


#endif
