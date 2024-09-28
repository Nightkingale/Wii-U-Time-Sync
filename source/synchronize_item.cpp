/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstdio>

#include <wupsxx/cafe_glyphs.h>
#include <wupsxx/logger.hpp>

#include "synchronize_item.hpp"

#include "cfg.hpp"
#include "core.hpp"


using namespace std::literals;

using namespace wups::config;
namespace logger = wups::logger;


synchronize_item::synchronize_item() :
    button_item{"Synchronize now!"}
{}


std::unique_ptr<synchronize_item>
synchronize_item::create()
{
    return std::make_unique<synchronize_item>();
}


void
synchronize_item::on_started()
{
    status_msg = "Synchronizing...";

    task_stopper = {};

    auto task = [this](std::stop_token token)
    {
        try {
            logger::guard lguard;
            core::run(token, true);
            current_state = state::stopped;
        }
        catch (std::exception& e) {
            current_state = state::stopped;
            throw;
        }
    };

    task_result = std::async(std::launch::async,
                             std::move(task),
                             task_stopper.get_token());
}


void
synchronize_item::on_finished()
{
    try {
        task_result.get();
        status_msg = "Success!";
        cfg::save_important_vars();
    }
    catch (std::exception& e) {
        logger::printf("ERROR: %s\n", e.what());
        status_msg = e.what();
    }
}


void
synchronize_item::on_cancel()
{
    task_stopper.request_stop();
}
