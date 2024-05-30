// SPDX-License-Identifier: MIT

#include "wupsxx/category.hpp"
#include "wupsxx/config_error.hpp"

namespace wups::config {

    category::category(WUPSConfigCategoryHandle handle) :
        handle{handle},
        own_handle{false}
    {}


    category::category(const std::string& label) :
        own_handle{true}
    {
        WUPSConfigAPICreateCategoryOptionsV1 options{ .name = label.c_str() };
        auto status = WUPSConfigAPI_Category_Create(options, &handle);
        if (status != WUPSCONFIG_API_RESULT_SUCCESS)
            throw config_error{status, "could not create category \"" + label + "\""};
    }


    category::category(category&& other)
        noexcept
    {
        handle = other.handle;
        other.handle = {};
    }


    category::~category()
    {
        if (own_handle && handle.handle)
            WUPSConfigAPI_Category_Destroy(handle);
    }


    void
    category::release()
    {
        handle = {};
    }


    void
    category::add(std::unique_ptr<item>&& item)
    {
        if (!item)
            throw std::logic_error{"cannot add null item to category"};
        if (!item->handle.handle)
            throw std::logic_error{"cannot add null item handle to category"};

        auto status = WUPSConfigAPI_Category_AddItem(handle, item->handle);
        if (status != WUPSCONFIG_API_RESULT_SUCCESS)
            throw config_error{status, "cannot add item to category: "};

        item.release(); // WUPS will call .onDelete() later
    }


    void
    category::add(category&& child)
    {
        auto status = WUPSConfigAPI_Category_AddCategory(handle, child.handle);
        if (status != WUPSCONFIG_API_RESULT_SUCCESS)
            throw config_error{status, "cannot add child category to category: "};

        child.release();
    }

} // namespace wups::config
