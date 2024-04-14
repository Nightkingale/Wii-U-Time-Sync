// SPDX-License-Identifier: MIT

#ifndef LOG_HPP
#define LOG_HPP

#include <string>

#include <whb/log.h>


#define LOG_PREFIX "[" PLUGIN_NAME "] "

#define LOG(FMT, ...)  WHBLogPrintf(LOG_PREFIX FMT __VA_OPT__(,) __VA_ARGS__)


void report_error(const std::string& arg);
void report_info(const std::string& arg);
void report_success(const std::string& arg);

#endif
