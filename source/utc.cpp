// SPDX-License-Identifier: MIT

#include <coreinit/time.h>

#include "utc.hpp"

#include "cfg.hpp"


namespace utc {

    static
    double
    local_time()
    {
        return static_cast<double>(OSGetTime()) / OSTimerClockSpeed;
    }


    timestamp
    now()
        noexcept
    {
        auto offset_seconds = duration_cast<std::chrono::seconds>(cfg::get_utc_offset());
        return timestamp{ local_time() - offset_seconds.count() };
    }

} // namespace utc
