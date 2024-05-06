// SPDX-License-Identifier: MIT

#include <stdexcept>

#include "wupsxx/config.hpp"


namespace wups {

    config::config(const std::string& name)
    {
        if (WUPSConfig_Create(&handle, name.c_str()) < 0)
            throw std::runtime_error{"could not create config"};
    }


    config::~config()
    {
        if (handle)
            WUPSConfig_Destroy(handle);
    }


    void
    config::add(std::unique_ptr<category>&& cat)
    {
        if (!cat)
            throw std::logic_error{"cannot add null category to config"};
        if (!cat->handle)
            throw std::logic_error{"cannot add null category handle to config"};

        if (WUPSConfig_AddCategory(handle, cat->handle) < 0)
            throw std::runtime_error{"cannot add category to config"};

        cat.release(); // WUPS now owns this category
    }

} // namespace wups
