// SPDX-License-Identifier: MIT

#ifndef WUPSXX_CONFIG_HPP
#define WUPSXX_CONFIG_HPP

#include <memory>
#include <string>

#include <wups.h>

#include "category.hpp"


namespace wups {

    struct config {

        WUPSConfigHandle handle = 0;

        config(const std::string& name);

        config(config&&) = delete;

        ~config();

        void add(std::unique_ptr<category>&& cat);

    };

} // namespace wups

#endif
