// SPDX-License-Identifier: MIT

#include <cmath>                // abs()
#include <cstdio>               // snprintf()

#include "time_utils.hpp"


using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::minutes;
using std::chrono::hours;

using namespace std::literals;


namespace time_utils {

    // to_string() functions that show the time unit

    template<>
    std::string
    to_string<milliseconds>(milliseconds t)
    { return std::to_string(t.count()) + " ms"; }

    template<>
    std::string
    to_string<seconds>(seconds t)
    { return std::to_string(t.count()) + " s"; }

    template<>
    std::string
    to_string<minutes>(minutes t)
    { return std::to_string(t.count()) + " min"; }

    template<>
    std::string
    to_string<hours>(hours t)
    { return std::to_string(t.count()) + " h"; }


    std::string
    seconds_to_human(dbl_seconds s, bool show_positive)
    {
        char buf[64];

        if (abs(s) < 2s)
            std::snprintf(buf, sizeof buf, "%.1f ms", 1000 * s.count());
        else if (abs(s) < 2min)
            std::snprintf(buf, sizeof buf, "%.1f s", s.count());
        else if (abs(s) < 2h)
            std::snprintf(buf, sizeof buf, "%.1f min", s.count() / 60);
        else if (abs(s) < 48h)
            std::snprintf(buf, sizeof buf, "%.1f hrs", s.count() / (60 * 60));
        else
            std::snprintf(buf, sizeof buf, "%.1f days", s.count() / (24 * 60 * 60));

        std::string result = buf;

        if (show_positive && s > 0s)
            result = "+" + result;

        return result;
    }


    std::string
    tz_offset_to_string(minutes offset)
    {
        char buf[32];
        char sign = offset < 0min ? '-' : '+';
        int hours = std::abs(offset.count() / 60);
        int minutes = std::abs(offset.count() % 60);
        std::snprintf(buf, sizeof buf, "%c%02d:%02d", sign, hours, minutes);
        return buf;
    }

} // namespace time_utils
