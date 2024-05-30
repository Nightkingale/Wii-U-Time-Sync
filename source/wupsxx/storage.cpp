// SPDX-License-Identifier: MIT

#include <wups/storage.h>

#include "wupsxx/storage.hpp"


namespace wups::storage {

    void
    save()
    {
        auto status = WUPSStorageAPI::SaveStorage();
        if (status != WUPS_STORAGE_ERROR_SUCCESS)
            throw storage_error{"error saving storage", status};
    }


    void
    reload()
    {
        auto status = WUPSStorageAPI::ForceReloadStorage();
        if (status != WUPS_STORAGE_ERROR_SUCCESS)
            throw storage_error{"error reloading storage", status};
    }


} // namespace wups::storage
