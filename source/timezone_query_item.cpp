// SPDX-License-Identifier: MIT

#include <cstdio>               // snprintf()
#include <exception>
#include <string.h>             // BSD strlcpy()

#include "timezone_query_item.hpp"

#include "cfg.hpp"
#include "logging.hpp"
#include "nintendo_glyphs.h"
#include "utils.hpp"
#include "wupsxx/storage.hpp"


using namespace std::literals;


timezone_query_item::timezone_query_item(const std::string& key,
                                         const std::string& label,
                                         int& variable,
                                         const int default_value) :
    wups::config::item{key, label},
    variable(variable),
    default_value{default_value},
    text{"Query "s + utils::get_tz_service_name(variable)}
{}


std::unique_ptr<timezone_query_item>
timezone_query_item::create(const std::string& key,
                            const std::string& label,
                            int& variable,
                            const int default_value)
{
    return std::make_unique<timezone_query_item>(key, label, variable, default_value);
}


int
timezone_query_item::get_display(char* buf, std::size_t size)
    const
{
    ::strlcpy(buf, text.c_str(), size);
    return 0;
}


int
timezone_query_item::get_selected_display(char* buf, std::size_t size)
    const
{
    std::snprintf(buf, size,
                  "%s %s %s",
                  NIN_GLYPH_BTN_DPAD_LEFT,
                  text.c_str(),
                  NIN_GLYPH_BTN_DPAD_RIGHT);
    return 0;
}


void
timezone_query_item::restore()
{
    variable = default_value;

    on_changed();
}


void
timezone_query_item::on_input(WUPSConfigSimplePadData input,
                              WUPS_CONFIG_SIMPLE_INPUT repeat)
{
    wups::config::item::on_input(input, repeat);

    const int n = utils::get_num_tz_services();

    if (input.buttons_d & WUPS_CONFIG_BUTTON_LEFT)
        --variable;

    if (input.buttons_d & WUPS_CONFIG_BUTTON_RIGHT)
        ++variable;

    // let it wrap around
    if (*variable < 0)
        variable += n;
    if (*variable >= n)
        variable -= n;

    on_changed();

    if (input.buttons_d & WUPS_CONFIG_BUTTON_A)
        run();
}


void
timezone_query_item::on_changed()
{
    if (!variable.changed())
        return;

    text = "Query "s + utils::get_tz_service_name(*variable);

    try {
        wups::storage::store(*key, *variable);
        variable.reset();
    }
    catch (std::exception& e) {
        logging::printf("Error storing %s: %s", key->c_str(), e.what());
    }
}


void
timezone_query_item::run()
{
    try {
        auto [name, offset] = utils::fetch_timezone(*variable);
        text = name;
        cfg::set_and_store_utc_offset(offset);
    }
    catch (std::exception& e) {
        text = "Error: "s + e.what();
    }
}
