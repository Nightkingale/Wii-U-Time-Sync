/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef NOTIFY_HPP
#define NOTIFY_HPP

#include <string>


namespace notify {


    enum class level : int {
        quiet = 0,
        normal = 1,
        verbose = 2
    };

    void initialize();

    void finalize();


    void error(level lvl, const std::string& arg);

    void info(level lvl, const std::string& arg);

    void success(level lvl, const std::string& arg);


    // RAII type to ensure it's intialized and finalized
    class guard {
        bool must_finalize;
    public:
        guard(bool init = true);
        ~guard();
        void release();
    };

}

#endif
