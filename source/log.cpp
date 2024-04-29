// SPDX-License-Identifier: MIT

#include <notifications/notifications.h>

#include "log.hpp"

#include "cfg.hpp"


void
report_error(const std::string& arg)
{
    LOG("ERROR: %s", arg.c_str());

    if (!cfg::notify)
        return;

    std::string msg = LOG_PREFIX + arg;
    NotificationModule_AddErrorNotificationEx(msg.c_str(),
                                              cfg::msg_duration,
                                              1,
                                              {255, 255, 255, 255},
                                              {160, 32, 32, 255},
                                              nullptr,
                                              nullptr);
}


void
report_info(const std::string& arg)
{
    LOG("INFO: %s", arg.c_str());

    if (!cfg::notify)
        return;

    std::string msg = LOG_PREFIX + arg;
    NotificationModule_AddInfoNotificationEx(msg.c_str(),
                                             cfg::msg_duration,
                                             {255, 255, 255, 255},
                                             {32, 32, 160, 255},
                                             nullptr,
                                             nullptr);
}


void
report_success(const std::string& arg)
{
    LOG("SUCCESS: %s", arg.c_str());

    if (!cfg::notify)
        return;

    std::string msg = LOG_PREFIX + arg;
    NotificationModule_AddInfoNotificationEx(msg.c_str(),
                                             cfg::msg_duration,
                                             {255, 255, 255, 255},
                                             {32, 160, 32, 255},
                                             nullptr,
                                             nullptr);
}
