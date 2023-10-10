// SPDX-License-Identifier: MIT

#include <cstdio>
#include <stdexcept>

#include "text_item.hpp"


using namespace std::literals;

namespace wups {

    text_item::text_item(const std::string& key,
                         const std::string& name,
                         const std::string& text) :
        base_item{key, name},
        text{text}
    {}


    int
    text_item::get_current_value_display(char* buf,
                                         std::size_t size)
        const
    {
        std::snprintf(buf, size, text.c_str());
        if (size > 3 && text.size() + 1 > size)
            // replace last 3 chars of buf with "..."
            buf[size - 2] = buf[size - 3] = buf[size - 4] = '.';
        return 0;
    }

} // namespace wups
