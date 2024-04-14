// SPDX-License-Identifier: MIT

#include <stdexcept>

#include "../../include/wupsxx/category.hpp"


namespace wups {

    category::category(const std::string& name)
    {
        if (WUPSConfigCategory_Create(&handle, name.c_str()) < 0)
            throw std::runtime_error{"could not create category"};
    }


    category::~category()
    {
        if (handle)
            WUPSConfigCategory_Destroy(handle);
    }


    void
    category::add(std::unique_ptr<base_item>&& item)
    {
        if (!item)
            throw std::logic_error{"cannot add null item to category"};
        if (!item->handle)
            throw std::logic_error{"cannot add null item handle to category"};

        if (WUPSConfigCategory_AddItem(handle, item->handle) < 0)
            throw std::runtime_error{"cannot add item to category"};

        item.release(); // WUPS now owns this item
    }


    void
    category::add(base_item* item)
    {
        if (!item)
            throw std::logic_error{"cannot add null item to category"};
        if (!item->handle)
            throw std::logic_error{"cannot add null item handle to category"};

        if (WUPSConfigCategory_AddItem(handle, item->handle) < 0)
            throw std::runtime_error{"cannot add item to category"};
    }


} // namespace wups
