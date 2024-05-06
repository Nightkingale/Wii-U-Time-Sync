// SPDX-License-Identifier: MIT

#ifndef UTC_HPP
#define UTC_HPP

namespace utc {

    extern double timezone_offset;


    // Seconds since 2000-01-01 00:00:00 UTC
    struct timestamp {
        double value;
    };


    timestamp now() noexcept;

} // namespace utc

#endif
