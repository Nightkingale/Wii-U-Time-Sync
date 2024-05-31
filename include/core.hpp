// SPDX-License-Identifier: MIT

#ifndef CORE_HPP
#define CORE_HPP

#include <string>
#include <utility>              // pair<>

#include "net/address.hpp"


namespace core {

    std::pair<double, double> ntp_query(net::address address);

    void run();

    std::string local_clock_to_string();

} // namespace core

#endif
