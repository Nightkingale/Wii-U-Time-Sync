// SPDX-License-Identifier: MIT

#ifndef TIME_UTILS_HPP
#define TIME_UTILS_HPP

#include <chrono>
#include <string>
#include <type_traits>


namespace time_utils {

    // Type-safe way to pass seconds around as double
    using dbl_seconds = std::chrono::duration<double>;


    // Type trait to identify when a type is std::chrono::duration<>

    template<typename T>
    struct is_duration : std::false_type {};

    template<typename R, typename P>
    struct is_duration<std::chrono::duration<R, P>> : std::true_type {};

    // convenience variable template
    template<typename T>
    constexpr bool is_duration_v = is_duration<T>::value;


    template<typename T>
    concept duration = is_duration_v<T>;


    template<duration T>
    std::string to_string(T t);



    // Generate time duration strings for humans.
    std::string seconds_to_human(dbl_seconds s, bool show_positive = false);


    std::string tz_offset_to_string(std::chrono::minutes offset);

}


#endif
