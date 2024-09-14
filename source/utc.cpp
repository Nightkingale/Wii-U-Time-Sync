/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include <coreinit/time.h>

#include "utc.hpp"

#include "cfg.hpp"


namespace utc {

    static
    dbl_seconds
    local_time()
    {
        double t = static_cast<double>(OSGetTime()) / OSTimerClockSpeed;
        return dbl_seconds{t};
    }


    timestamp
    now()
        noexcept
    {
        return timestamp{ local_time() - cfg::utc_offset };
    }

} // namespace utc
