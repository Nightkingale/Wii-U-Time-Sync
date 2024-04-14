// SPDX-License-Identifier: MIT

#ifndef WUPSXX_CATEGORY_HPP
#define WUPSXX_CATEGORY_HPP

#include <memory>
#include <string>

#include <wups.h>

#include "base_item.hpp"


namespace wups {

    struct category {

        WUPSConfigCategoryHandle handle = 0;

        category(const std::string& name);

        category(category&&) = delete;

        ~category();

        void add(std::unique_ptr<base_item>&& item);

        void add(base_item* item);

    };

} // namespace wups


#endif
