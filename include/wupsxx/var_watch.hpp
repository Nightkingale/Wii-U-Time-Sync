#ifndef WUPSXX_VAR_WATCH_HPP
#define WUPSXX_VAR_WATCH_HPP

#include <utility>              // forward()


// A helper class that tracks changes to a variable.

namespace wups::config {

    template<typename T>
    class var_watch {

        T& ref;
        bool modified = false;

    public:

        var_watch(T& var) :
            ref(var)
        {}


        bool
        changed()
            const noexcept
        {
            return modified;
        }


        void
        reset()
            noexcept
        {
            modified = false;
        }


        template<typename U>
        var_watch&
        operator =(U&& val)
            noexcept
        {
            T old = ref;
            ref = std::forward<U>(val);
            if (old != ref)
                modified = true;
            return *this;
        }


        // operator T()
        //     const noexcept
        // {
        //     return ref;
        // }


        T
        value()
            const noexcept
        {
            return ref;
        }


        // pointer-like read-only accessors

        const T&
        operator *()
            const noexcept
        {
            return ref;
        }


        const T*
        operator ->()
            const noexcept
        {
            return &ref;
        }


        // modifier below this point


        // increment/decrement
        var_watch&
        operator ++()
            noexcept
        {
            T old = ref;
            ++ref;
            if (old != ref)
                modified = true;
            return *this;
        }


        var_watch&
        operator --()
            noexcept
        {
            T old = ref;
            --ref;
            if (old != ref)
                modified = true;
            return *this;
        }


        T
        operator ++(int)
            noexcept
        {
            T old = ref;
            T result = ref++;
            if (old != ref)
                modified = true;
            return result;
        }


        T
        operator --(int)
            noexcept
        {
            T old = ref;
            T result = ref--;
            if (old != ref)
                modified = true;
            return result;
        }


        // assignment modifiers

        template<typename U>
        var_watch&
        operator +=(U&& arg)
            noexcept
        {
            T old = ref;
            ref += std::forward<U>(arg);
            if (old != ref)
                modified = true;
            return *this;
        }


        template<typename U>
        var_watch&
        operator -=(U&& arg)
            noexcept
        {
            T old = ref;
            ref -= std::forward<U>(arg);
            if (old != ref)
                modified = true;
            return *this;
        }


        template<typename U>
        var_watch&
        operator *=(U&& arg)
            noexcept
        {
            T old = ref;
            ref *= std::forward<U>(arg);
            if (old != ref)
                modified = true;
            return *this;
        }


        template<typename U>
        var_watch&
        operator /=(U&& arg)
            noexcept
        {
            T old = ref;
            ref /= std::forward<U>(arg);
            if (old != ref)
                modified = true;
            return *this;
        }


        template<typename U>
        var_watch&
        operator %=(U&& arg)
            noexcept
        {
            T old = ref;
            ref %= std::forward<U>(arg);
            if (old != ref)
                modified = true;
            return *this;
        }


        template<typename U>
        var_watch&
        operator <<=(U&& arg)
            noexcept
        {
            T old = ref;
            ref <<= std::forward<U>(arg);
            if (old != ref)
                modified = true;
            return *this;
        }


        template<typename U>
        var_watch&
        operator >>=(U&& arg)
            noexcept
        {
            T old = ref;
            ref >>= std::forward<U>(arg);
            if (old != ref)
                modified = true;
            return *this;
        }


        template<typename U>
        var_watch&
        operator ^=(U&& arg)
            noexcept
        {
            T old = ref;
            ref ^= std::forward<U>(arg);
            if (old != ref)
                modified = true;
            return *this;
        }


        template<typename U>
        var_watch&
        operator &=(U&& arg)
            noexcept
        {
            T old = ref;
            ref &= std::forward<U>(arg);
            if (old != ref)
                modified = true;
            return *this;
        }


        template<typename U>
        var_watch&
        operator |=(U&& arg)
            noexcept
        {
            T old = ref;
            ref |= std::forward<U>(arg);
            if (old != ref)
                modified = true;
            return *this;
        }


    };

}


#endif
