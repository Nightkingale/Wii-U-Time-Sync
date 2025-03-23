/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>               // snprintf()
#include <exception>
#include <string.h>             // BSD strlcpy()

#include <wupsxx/cafe_glyphs.h>
#include <wupsxx/storage.hpp>

#include "time_zone_query_item.hpp"

#include "cfg.hpp"
#include "utils.hpp"


using namespace std::literals;


namespace {
    std::string
    make_query_text(int idx)
    {
        return "Query "s + utils::get_tz_service_name(idx);
    }
} // namespace


time_zone_query_item::time_zone_query_item(wups::option<int>& opt) :
    var_item{opt},
    text{make_query_text(opt.value)}
{}


std::unique_ptr<time_zone_query_item>
time_zone_query_item::create(wups::option<int>& opt)
{
    return std::make_unique<time_zone_query_item>(opt);
}


void
time_zone_query_item::get_display(char* buf, std::size_t size)
    const
{
    ::strlcpy(buf, text.c_str(), size);
}


void
time_zone_query_item::get_focused_display(char* buf, std::size_t size)
    const
{
    std::snprintf(buf, size,
                  "%s %s %s",
                  CAFE_GLYPH_BTN_LEFT,
                  make_query_text(variable).c_str(),
                  CAFE_GLYPH_BTN_RIGHT);
}


wups::focus_status
time_zone_query_item::on_input(const wups::simple_pad_data& input)
{
    const int n = utils::get_num_tz_services();

    auto prev_variable = variable;

    if (input.buttons_d & WUPS_CONFIG_BUTTON_LEFT)
        --variable;

    if (input.buttons_d & WUPS_CONFIG_BUTTON_RIGHT)
        ++variable;

    // let it wrap around
    if (variable < 0)
        variable += n;
    if (variable >= n)
        variable -= n;

    if (prev_variable != variable)
        text = make_query_text(variable);

    if (input.buttons_d & WUPS_CONFIG_BUTTON_X) {
        restore_default();
        text = make_query_text(variable);
        return wups::focus_status::lose;
    }

    if (input.buttons_d & WUPS_CONFIG_BUTTON_B) {
        cancel_change();
        text = make_query_text(variable);
        return wups::focus_status::lose;
    }

    if (input.buttons_d & WUPS_CONFIG_BUTTON_A) {
        confirm_change();
        run();
        return wups::focus_status::lose;
    }

    return wups::focus_status::keep;
}


void
time_zone_query_item::run()
{
    try {
        auto [name, offset] = utils::fetch_timezone(variable);
        text = name;
        cfg::set_and_store_utc_offset(offset);
    }
    catch (std::exception& e) {
        text = "Error: "s + e.what();
    }
}
