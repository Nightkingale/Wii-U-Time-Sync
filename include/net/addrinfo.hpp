// SPDX-License-Identifier: MIT

#ifndef NET_ADDRINFO_HPP
#define NET_ADDRINFO_HPP

#include <optional>
#include <string>
#include <vector>

#include <netdb.h>

#include "address.hpp"
#include "socket.hpp"

// Note: Wii U only supports IPv4, so this is hardcoded for IPv4

namespace net::addrinfo {

    struct hints {
        std::optional<socket::type> type;
        bool canon_name   = false;
        bool numeric_host = false;
        bool passive      = false;
    };


    struct result {
        socket::type type;
        address      addr;
        std::optional<std::string> canon_name;
    };


    std::vector<result>
    lookup(const std::optional<std::string>& name,
           const std::optional<std::string>& service = {},
           std::optional<hints> options = {});



} // namespace net::addrinfo

#endif
