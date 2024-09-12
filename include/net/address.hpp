/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef NET_ADDRESS_HPP
#define NET_ADDRESS_HPP

#include <compare>
#include <string>
#include <vector>

#include <netinet/in.h>         // sockaddr_in, in_addr_t, in_port_t


// Note: only IPv4 address families, the Wii U does not support IPv6

namespace net {

    using ipv4_t = in_addr_t;
    using port_t = in_port_t;


    // Note: this is small enough, you can pass it by value everywhere.

    struct address {

        ipv4_t ip = 0;
        port_t port = 0;


        constexpr
        address() noexcept = default;

        address(const sockaddr_in& src) noexcept;

        address(ipv4_t ip, port_t port) noexcept;

        address(const sockaddr* ptr, socklen_t size);


        sockaddr_in
        data() const noexcept;


        // let the compiler generate comparisons
        constexpr bool operator ==(const address&) const noexcept = default;
        constexpr std::strong_ordering operator <=>(const address&) const noexcept = default;

    };


    std::string to_string(address addr);


} // namespace net

#endif
