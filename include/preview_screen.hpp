// SPDX-License-Identifier: MIT

#ifndef PREVIEW_SCREEN_HPP
#define PREVIEW_SCREEN_HPP

#include <map>
#include <string>

#include "wupsxx/category.hpp"
#include "wupsxx/text_item.hpp"


struct preview_screen : wups::category {

    struct server_info {
        wups::text_item* name = nullptr;
        wups::text_item* correction = nullptr;
        wups::text_item* latency = nullptr;
    };


    wups::text_item* clock = nullptr;
    std::map<std::string, server_info> server_infos;


    preview_screen();

    void run();

};


#endif
