// SPDX-License-Identifier: MIT

#ifndef LOGGING_HPP
#define LOGGING_HPP


namespace logging {

    void initialize();

    void finalize();

    __attribute__(( __format__ (__printf__, 1, 2) ))
    void printf(const char* fmt, ...);

} // namespace logging

#endif
