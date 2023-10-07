// SPDX-License-Identifier: MIT

#ifndef NTP_HPP
#define NTP_HPP

#include <cstdint>


namespace ntp {
    // For details, see https://www.ntp.org/reflib/rfc/rfc5905.txt

    // This is u32.32 fixed-point format, seconds since 1900-01-01.
    using timestamp = std::uint64_t;

    // This is a u16.16 fixed-point format.
    using short_timestamp = std::uint32_t;


    // Note: all fields are big-endian
    struct packet {

        enum class leap : std::uint8_t {
            no_warning      = 0 << 6,
            one_more_second = 1 << 6,
            one_less_second = 2 << 6,
            unknown         = 3 << 6
        };

        enum class mode : std::uint8_t {
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

        timestamp reference_time = 0; // Reference timestamp.
        timestamp origin_time    = 0; // Origin timestamp, aka T1.
        timestamp receive_time   = 0; // Receive timestamp, aka T2.
        timestamp transmit_time  = 0; // Transmit timestamp, aka T3.


        void leap(leap x)
        {
            lvm = static_cast<std::uint8_t>(x) | (lvm & 0b0011'1111);
        }

        void version(unsigned v)
        {
            lvm = ((v << 3) & 0b0011'1000) | (lvm & 0b1100'0111);
        }

        void mode(mode m)
        {
            lvm = static_cast<std::uint8_t>(m) | (lvm & 0b1111'1000);
        }

    };

    static_assert(sizeof(packet) == 48);

} // namespace ntp


#endif
