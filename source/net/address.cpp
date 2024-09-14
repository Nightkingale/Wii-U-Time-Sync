/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#include <cstring>              // memset()
#include <stdexcept>

#include <arpa/inet.h>          // htons(), inet_ntop()
#include <sys/socket.h>         // AF_INET

#include "net/address.hpp"


namespace net {

    address::address(const sockaddr_in& src)
        noexcept
    {
        ip = ntohl(src.sin_addr.s_addr);
        port = ntohs(src.sin_port);
    }


    address::address(ipv4_t ip, port_t port)
        noexcept :
        ip{ip},
        port{port}
    {}


    address::address(const sockaddr* ptr, socklen_t size)
    {
        if (size != sizeof(sockaddr_in))
            throw std::logic_error{"address size mismatch"};
        sockaddr_in src;
        std::memcpy(&src, ptr, size);
        ip = ntohl(src.sin_addr.s_addr);
        port = ntohs(src.sin_port);
    }


    sockaddr_in
    address::data()
        const noexcept
    {
        sockaddr_in result;

        result.sin_family = AF_INET;
        result.sin_port = htons(port);
        result.sin_addr.s_addr = htonl(ip);
        std::memset(result.sin_zero, 0, sizeof result.sin_zero);

        return result;
    }


    std::string
    to_string(address addr)
    {
        auto raw_addr = addr.data();
        char buf[INET_ADDRSTRLEN];
        return inet_ntop(AF_INET, &raw_addr.sin_addr, buf, sizeof buf);
    }


} // namespace net
