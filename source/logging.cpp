// SPDX-License-Identifier: MIT

#include <atomic>
#include <cstdarg>
#include <cstdio>               // vsnprintf()
#include <string>

#include <whb/log.h>
#include <whb/log_cafe.h>
#include <whb/log_module.h>
#include <whb/log_udp.h>

#include "logging.hpp"


using namespace std::literals;


namespace logging {

    std::atomic_uint refs = 0;


    bool init_cafe = false;
    bool init_module = false;
    bool init_udp = false;


    void
    initialize()
    {
        // don't initialize again if refs was already positive
        if (refs++)
            return;

        init_cafe = WHBLogCafeInit();
        init_module = WHBLogModuleInit();
        init_udp = WHBLogUdpInit();
    }


    void
    finalize()
    {
        // don't finalize if refs is still positive
        if (--refs)
            return;

        if (init_cafe)
            WHBLogCafeDeinit();
        init_cafe = false;

        if (init_module)
            WHBLogModuleDeinit();
        init_module = false;

        if (init_udp)
            WHBLogUdpDeinit();
        init_udp = false;
    }


    void
    printf(const char* fmt, ...)
    {
        std::string buf(256, '\0');
        std::string xfmt = std::string("[" PLUGIN_NAME "] ") + fmt;

        std::va_list args;

        va_start(args, fmt);
        int sz = std::vsnprintf(buf.data(), buf.size(), xfmt.c_str(), args);
        va_end(args);

        if (sz > 0 && static_cast<unsigned>(sz) >= buf.size()) {
            buf.resize(sz + 1);

            va_start(args, fmt);
            std::vsnprintf(buf.data(), buf.size(), xfmt.c_str(), args);
            va_end(args);
        }

        if (sz > 0)
            WHBLogPrint(buf.c_str());
    }

} // namespace logging
