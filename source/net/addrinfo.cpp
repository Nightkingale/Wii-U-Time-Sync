// SPDX-License-Identifier: MIT

#include <cstring> // memcpy(), memset()
#include <memory>
#include <stdexcept>

#include "net/addrinfo.hpp"


namespace net::addrinfo {

    struct addrinfo_deleter {
        void operator ()(struct ::addrinfo* p)
            const noexcept
        { ::freeaddrinfo(p); }
    };

    using ai_ptr = std::unique_ptr<struct ::addrinfo, addrinfo_deleter>;


    int
    to_flags(const hints& opt)
    {
        int flags = 0;
        if (opt.canon_name)
            flags |= AI_CANONNAME;
        if (opt.numeric_host)
            flags |= AI_NUMERICHOST;
        if (opt.passive)
            flags |= AI_PASSIVE;
        return flags;
    }


    socket::type
    to_type(int socktype, int protocol)
    {
        if (socktype == SOCK_STREAM && protocol == IPPROTO_TCP)
            return socket::type::tcp;
        if (socktype == SOCK_DGRAM && protocol == IPPROTO_UDP)
            return socket::type::udp;
        return {};
    }


    std::vector<result>
    lookup(const std::optional<std::string>& name,
           const std::optional<std::string>& service,
           std::optional<hints> opts)
    {
        ai_ptr info;

        struct ::addrinfo raw_hints;
        struct ::addrinfo* raw_hints_ptr = nullptr;

        if (opts) {
            raw_hints_ptr = &raw_hints;
            std::memset(&raw_hints, 0, sizeof raw_hints);
            raw_hints.ai_family = AF_INET;
            raw_hints.ai_flags = to_flags(*opts);
            if (opts->type) {
                switch (*opts->type) {
                case socket::type::tcp:
                    raw_hints.ai_socktype = SOCK_STREAM;
                    raw_hints.ai_protocol = IPPROTO_TCP;
                    break;
                case socket::type::udp:
                    raw_hints.ai_socktype = SOCK_DGRAM;
                    raw_hints.ai_protocol = IPPROTO_UDP;
                    break;
                }
            }
        }

        struct ::addrinfo* raw_result_ptr = nullptr;
        int status = ::getaddrinfo(name ? name->c_str() : nullptr,
                                   service ? service->c_str() : nullptr,
                                   raw_hints_ptr,
                                   &raw_result_ptr);
        if (status)
            throw std::runtime_error{::gai_strerror(status)};

        info.reset(raw_result_ptr);

        std::vector<result> res;

        // walk through the linked list
        for (auto a = info.get(); a; a = a->ai_next) {
            // sanity check: Wii U only supports IPv4
            if (a->ai_addrlen != sizeof(sockaddr_in))
                throw std::logic_error{"getaddrinfo() returned invalid result!"};

            result item;
            item.type = to_type(a->ai_socktype, a->ai_protocol);
            item.addr = address(a->ai_addr, a->ai_addrlen);
            if (a->ai_canonname)
                item.canon_name = std::string(a->ai_canonname);

            res.push_back(std::move(item));
        }

        return res;
    }

} // namespace net::addrinfo
