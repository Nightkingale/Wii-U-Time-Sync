/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include <bit>                  // endian, byteswap()
#include <cmath>                // ldexp()

#include <sys/endian.h>         // be64toh(), htobe64()

#include "ntp.hpp"


namespace ntp {

    timestamp::timestamp(dbl_seconds s)
        noexcept
    {
        // shift s to the left by 32 bits, to line up the fixed point
        std::uint64_t shifted_s = std::ldexp(s.count(), 32);
        store(shifted_s);
    }


    timestamp::operator dbl_seconds()
        const noexcept
    {
        // shift to the right by 32 bits
        double s = std::ldexp(static_cast<double>(load()), -32);
        return dbl_seconds{s};
    }


    std::uint64_t
    timestamp::load()
        const noexcept
    {
        return be64toh(stored);
    }


    void
    timestamp::store(std::uint64_t v)
        noexcept
    {
        stored = htobe64(v);
    }


    std::strong_ordering
    timestamp::operator <=>(timestamp other)
        const noexcept
    {
        return load() <=> other.load();
    }


    std::string
    to_string(packet::mode_flag m)
    {
        switch (m) {
        case packet::mode_flag::reserved:
            return "reserved";
        case packet::mode_flag::active:
            return "active";
        case packet::mode_flag::passive:
            return "passive";
        case packet::mode_flag::client:
            return "client";
        case packet::mode_flag::server:
            return "server";
        case packet::mode_flag::broadcast:
            return "broadcast";
        case packet::mode_flag::control:
            return "control";
        case packet::mode_flag::reserved_private:
            return "reserved_private";
        default:
            return "error";
        }
    }


    void
    packet::leap(leap_flag x)
        noexcept
    {
        lvm = static_cast<std::uint8_t>(x) | (lvm & 0b0011'1111);
    }


    packet::leap_flag
    packet::leap()
        const noexcept
    {
        return static_cast<leap_flag>((lvm & 0b1100'0000) >> 6);
    }


    void
    packet::version(unsigned v)
        noexcept
    {
        lvm = ((v << 3) & 0b0011'1000) | (lvm & 0b1100'0111);
    }


    unsigned
    packet::version()
        const noexcept
    {
        return (lvm & 0b0011'1000) >> 3;
    }


    void
    packet::mode(packet::mode_flag m)
        noexcept
    {
        lvm = static_cast<std::uint8_t>(m) | (lvm & 0b1111'1000);
    }


    packet::mode_flag
    packet::mode()
        const noexcept
    {
        return static_cast<mode_flag>(lvm & 0b000'0111);
    }

} // namespace ntp
