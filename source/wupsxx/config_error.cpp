// SPDX-License-Identifier: MIT

#include <wups/config_api.h>

#include "wupsxx/config_error.hpp"


using namespace std::literals;


namespace wups::config {

    config_error::config_error(WUPSConfigAPIStatus status,
                               const std::string& msg) :
        std::runtime_error{msg + ": "s + WUPSConfigAPI_GetStatusStr(status)}
    {}

} // namespace wups::config
