// SPDX-License-Identifier: MIT

#include "timezone_query_item.hpp"

#include "cfg.hpp"
#include "nintendo_glyphs.h"
#include "utils.hpp"


using namespace std::literals;


timezone_query_item::timezone_query_item() :
    wups::config::text_item{{},
                            "Detect Time Zone (press " NIN_GLYPH_BTN_A ")",
                            "using http://ip-api.com",
                            30}
{}


std::unique_ptr<timezone_query_item>
timezone_query_item::create()
{
    return std::make_unique<timezone_query_item>();
}


void
timezone_query_item::on_input(WUPSConfigSimplePadData input,
                              WUPS_CONFIG_SIMPLE_INPUT repeat)
{
    text_item::on_input(input, repeat);

    if (input.buttons_d & WUPS_CONFIG_BUTTON_A)
        run();
}


void
timezone_query_item::run()
{
    try {
        auto [name, offset] = utils::fetch_timezone();
        text = name;
        cfg::set_utc_offset(offset);
    }
    catch (std::exception& e) {
        text = "Error: "s + e.what();
    }
}
