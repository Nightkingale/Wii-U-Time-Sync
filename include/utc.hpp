// SPDX-License-Identifier: MIT

#ifndef UTC_HPP
#define UTC_HPP

#include "time_utils.hpp"


namespace utc {

    using time_utils::dbl_seconds;


    // Seconds since 2000-01-01 00:00:00 UTC
    struct timestamp {
        dbl_seconds value;
    };


    timestamp now() noexcept;

} // namespace utc

#endif
