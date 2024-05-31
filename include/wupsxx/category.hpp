// SPDX-License-Identifier: MIT

#ifndef WUPSXX_CATEGORY_HPP
#define WUPSXX_CATEGORY_HPP

#include <memory>
#include <string>

#include <wups.h>

#include "item.hpp"


namespace wups::config {

    class category final {

        WUPSConfigCategoryHandle handle;
        bool own_handle; // if true, will destroy the handle in the destructor

    public:

        // This constructor does not take ownership of the handle.
        category(WUPSConfigCategoryHandle handle);

        category(const std::string& label);
        category(category&& other) noexcept;

        ~category();

        void release();

        void add(std::unique_ptr<item>&& item);

        void add(category&& child);

    };

} // namespace wups

#endif
