// SPDX-License-Identifier: MIT

#ifndef NTP_HPP
#define NTP_HPP

#include <compare>
#include <cstdint>
#include <string>

#include "time_utils.hpp"


// For details, see https://www.ntp.org/reflib/rfc/rfc5905.txt

namespace ntp {

    using time_utils::dbl_seconds;


    // This is u32.32 fixed-point format, seconds since 1900-01-01 00:00:00 UTC
    class timestamp {

        std::uint64_t stored = 0; // in big-endian format

    public:

        constexpr timestamp() noexcept = default;

        timestamp(std::uint64_t v) = delete;

        // Allow explicit conversions from/to dbl_seconds
        explicit timestamp(dbl_seconds d) noexcept;
        explicit operator dbl_seconds() const noexcept;

        // Checks if timestamp is non-zero. Zero has a special meaning.
        constexpr explicit operator bool() const noexcept { return stored; }


        // These will byteswap if necessary.
        std::uint64_t load() const noexcept;
        void store(std::uint64_t v) noexcept;


        constexpr
        bool operator ==(const timestamp& other) const noexcept = default;

        std::strong_ordering operator <=>(timestamp other) const noexcept;

    };
    // TODO: implement difference calculations for timestamps, as recommended by the RFC.
    // Differences sould be done in fixed-point, should check for overflows, and return
    // floating-point.


    // This is a u16.16 fixed-point format.
    using short_timestamp = std::uint32_t;


    // Note: all fields are big-endian
    struct packet {

        enum class leap_flag : std::uint8_t {
            no_warning      = 0 << 6,
            one_more_second = 1 << 6,
            one_less_second = 2 << 6,
            unknown         = 3 << 6
        };

        enum class mode_flag : std::uint8_t {
            reserved         = 0,
            active           = 1,
            passive          = 2,
            client           = 3,
            server           = 4,
            broadcast        = 5,
            control          = 6,
            reserved_private = 7
        };


        // Note: all fields are zero-initialized by default constructor.
        std::uint8_t lvm           = 0; // leap, version and mode
        std::uint8_t stratum       = 0; // Stratum level of the local clock.
        std::int8_t  poll_exp      = 0; // Maximum interval between successive messages.
        std::int8_t  precision_exp = 0; // Precision of the local clock.

        short_timestamp root_delay      = 0; // Total round trip delay time to the reference clock.
        short_timestamp root_dispersion = 0; // Total dispersion to the reference clock.
        char            reference_id[4] = {0, 0, 0, 0}; // Reference clock identifier.

        timestamp reference_time; // Reference timestamp.
        timestamp origin_time;    // Origin timestamp.
        timestamp receive_time;   // Receive timestamp.
        timestamp transmit_time;  // Transmit timestamp.


        void leap(leap_flag x) noexcept;
        leap_flag leap() const noexcept;


        void version(unsigned v) noexcept;
        unsigned version() const noexcept;


        void mode(mode_flag m) noexcept;
        mode_flag mode() const noexcept;

    };

    static_assert(sizeof(packet) == 48);


    std::string to_string(packet::mode_flag m);

} // namespace ntp

#endif
