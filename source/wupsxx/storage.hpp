// SPDX-License-Identifier: MIT

#ifndef WUPSXX_STORAGE_HPP
#define WUPSXX_STORAGE_HPP

#include <string>
#include <expected>


namespace wups {

    template<typename T>
    std::expected<T, WUPSStorageError>
    load(const std::string& key);

    template<typename T>
    void store(const std::string& key, const T& value);

} // namespace wups

#endif
