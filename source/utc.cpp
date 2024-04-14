// SPDX-License-Identifier: MIT

#include <coreinit/time.h>

#include "../include/utc.hpp"


namespace utc {


    double timezone_offset = 0;


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
        return timestamp{ local_time() - timezone_offset };
    }


} // namespace utc
