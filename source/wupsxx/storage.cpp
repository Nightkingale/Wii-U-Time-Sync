// SPDX-License-Identifier: MIT

#include <wups.h>

#include "../../include/wupsxx/storage.hpp"


namespace wups {

    template<>
    std::expected<bool, WUPSStorageError>
    load<bool>(const std::string& key)
    {
        bool value;
        auto err = WUPS_GetBool(nullptr, key.c_str(), &value);
        if (err != WUPS_STORAGE_ERROR_SUCCESS)
            return std::unexpected{err};
        return value;
    }


    template<>
    std::expected<int, WUPSStorageError>
    load<int>(const std::string& key)
    {
        int value;
        auto err = WUPS_GetInt(nullptr, key.c_str(), &value);
        if (err != WUPS_STORAGE_ERROR_SUCCESS)
            return std::unexpected{err};
        return value;
    }


    template<>
    std::expected<std::string, WUPSStorageError>
    load<std::string>(const std::string& key)
    {
        // Note: we don't have WUPS_GetSize() so we can't know how big this has to be.
        std::string value(1024, '\0');
        auto err = WUPS_GetString(nullptr, key.c_str(), value.data(), value.size());
        if (err != WUPS_STORAGE_ERROR_SUCCESS)
            return std::unexpected{err};
        auto end = value.find('\0');
        return value.substr(0, end);
    }


    template<>
    void
    store<bool>(const std::string& key,
                const bool& value)
    {
        auto err = WUPS_StoreBool(nullptr, key.c_str(), value);
        if (err != WUPS_STORAGE_ERROR_SUCCESS)
            throw err;
    }


    template<>
    void
    store<int>(const std::string& key,
               const int& value)
    {
        auto err = WUPS_StoreInt(nullptr, key.c_str(), value);
        if (err != WUPS_STORAGE_ERROR_SUCCESS)
            throw err;
    }


    template<>
    void
    store<std::string>(const std::string& key,
                       const std::string& value)
    {
        auto err = WUPS_StoreString(nullptr, key.c_str(), value.c_str());
        if (err != WUPS_STORAGE_ERROR_SUCCESS)
            throw err;
    }

} // namespace wups
