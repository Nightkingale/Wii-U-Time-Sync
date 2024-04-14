// SPDX-License-Identifier: MIT

#include <memory>               // make_unique()

#include "wupsxx/bool_item.hpp"
#include "wupsxx/int_item.hpp"
#include "wupsxx/text_item.hpp"

#include "config_screen.hpp"

#include "../include/cfg.hpp"
#include "../include/http_client.hpp"
#include "../include/nintendo_glyphs.hpp"
#include "../include/utils.hpp"


using wups::bool_item;
using wups::int_item;
using wups::text_item;
using std::make_unique;

using namespace std::literals;


struct timezone_item : wups::text_item {

    timezone_item() :
        wups::text_item{"",
                        "Detect Timezone (press " NIN_GLYPH_BTN_A ")",
                        "Using http://ip-api.com"}
    {}


    void
    on_button_pressed(WUPSConfigButtons buttons)
        override
    {
        text_item::on_button_pressed(buttons);

        if (buttons & WUPS_CONFIG_BUTTON_A)
            query_timezone();
    }


    void
    query_timezone()
    try {
        std::string tz = http::get("http://ip-api.com/line/?fields=timezone,offset");
        auto tokens = utils::split(tz, " \r\n");
        if (tokens.size() != 2)
            throw std::runtime_error{"Could not parse response from \"ip-api.com\"."};

        int tz_offset = std::stoi(tokens[1]);
        text = tokens[0];

        cfg::hours = tz_offset / (60 * 60);
        cfg::minutes = tz_offset % (60 * 60) / 60;
        if (cfg::minutes < 0) {
            cfg::minutes += 60;
            --cfg::hours;
        }
    }
    catch (std::exception& e) {
        text = "Error: "s + e.what();
    }

};


config_screen::config_screen() :
    wups::category{"Configuration"}
{
    add(make_unique<bool_item>(cfg::key::sync, "Syncing Enabled", cfg::sync));
    add(make_unique<bool_item>(cfg::key::notify, "Show Notifications", cfg::notify));
    add(make_unique<int_item>(cfg::key::hours, "Hours Offset", cfg::hours, -12, 14));
    add(make_unique<int_item>(cfg::key::minutes, "Minutes Offset", cfg::minutes, 0, 59));
    add(make_unique<int_item>(cfg::key::msg_duration, "Notification Duration (seconds)",
                              cfg::msg_duration, 0, 30));
    add(make_unique<int_item>(cfg::key::tolerance, "Tolerance (milliseconds)",
                              cfg::tolerance, 0, 5000));

    add(make_unique<timezone_item>());

    // show current NTP server address, no way to change it.
    add(make_unique<wups::text_item>(cfg::key::server, "NTP Servers", cfg::server));
}
