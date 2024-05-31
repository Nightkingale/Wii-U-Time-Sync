// SPDX-License-Identifier: MIT

#include <utility>              // move()

#include "preview_screen.hpp"

#include "cfg.hpp"
#include "clock_item.hpp"
#include "utils.hpp"
#include "wupsxx/text_item.hpp"


using wups::config::text_item;


/*
 * Note: the clock item needs to know about the server items added later.
 * It's a bit ugly, because we can't manage it from the category object.
 */
wups::config::category
make_preview_screen()
{
    wups::config::category cat{"Preview Time"};

    auto clock = clock_item::create();
    auto& server_infos = clock->server_infos;

    cat.add(std::move(clock));

    auto servers = utils::split(cfg::server, " \t,;");
    for (const auto& server : servers) {
        if (!server_infos.contains(server)) {
            auto& si = server_infos[server];

            auto name = text_item::create({}, server + ":");
            si.name = name.get();
            cat.add(std::move(name));

            auto correction = text_item::create({}, "┣ Correction:", "", 48);
            si.correction = correction.get();
            cat.add(std::move(correction));

            auto latency = text_item::create({}, "┗ Latency:");
            si.latency = latency.get();
            cat.add(std::move(latency));
        }
    }

    return cat;
}
