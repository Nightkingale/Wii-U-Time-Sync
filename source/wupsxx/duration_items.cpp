// SPDX-License-Identifier: MIT

#include "numeric_item_impl.hpp"

namespace wups::config {

    template class numeric_item<std::chrono::milliseconds>;

    template class numeric_item<std::chrono::seconds>;

} // namespace wups::config
