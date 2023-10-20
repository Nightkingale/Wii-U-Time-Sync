// SPDX-License-Identifier: MIT

#ifndef CORE_HPP
#define CORE_HPP

#include <utility>              // pair<>
#include <string>

#include <netinet/in.h>         // struct sockaddr_in


namespace core {

    std::pair<double, double> ntp_query(struct sockaddr_in address);

    void sync_clock();

    std::string local_clock_to_string();

} // namespace core


#endif
