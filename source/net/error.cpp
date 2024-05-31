// SPDX-License-Identifier: MIT

#include "net/error.hpp"

#include "utils.hpp"


namespace net {

    error::error(int code, const std::string& msg) :
        std::system_error{std::make_error_code(std::errc{code}), msg}
    {}


    error::error(int code) :
        std::system_error{std::make_error_code(std::errc{code})}
    {}

} // namespace std
