// SPDX-License-Identifier: MIT

#ifndef WUPSXX_DURATION_ITEMS_HPP
#define WUPSXX_DURATION_ITEMS_HPP

#include <chrono>

#include "numeric_item.hpp"


namespace wups::config {

    using milliseconds_item = numeric_item<std::chrono::milliseconds>;

    using seconds_item = numeric_item<std::chrono::seconds>;

} // namespace wups::config

#endif
