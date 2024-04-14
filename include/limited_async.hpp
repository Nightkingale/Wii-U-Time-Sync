// SPDX-License-Identifier: MIT

#include <functional>           // invoke()
#include <future>
#include <semaphore>


/*
 * This is an implementation of a wrapper for std::async() that limits the number of
 * concurrent threads. Any call beyond the limit will block, so make sure the function
 * argument does not block indefinitely.
 *
 * This is needed because the Wii U's socket implementation can only handle a small
 * number of concurrent threads.
 */

enum class guard_type {
    acquire_and_release,
    only_acquire,
    only_release
};


template<typename Sem>
struct semaphore_guard {
    Sem& sem;
    guard_type type;

    semaphore_guard(Sem& s,
                    guard_type t = guard_type::acquire_and_release) :
        sem(s),
        type{t}
    {
        if (type == guard_type::acquire_and_release ||
            type == guard_type::only_acquire)
            sem.acquire();
    }

    ~semaphore_guard()
    {
        if (type == guard_type::acquire_and_release ||
            type == guard_type::only_release)
            sem.release();
    }
};


extern std::counting_semaphore<> async_limit;


template<typename Func,
         typename... Args>
[[nodiscard]]
std::future<typename std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>>
limited_async(Func&& func,
              Args&&... args)
{

    semaphore_guard caller_guard{async_limit}; // acquire the semaphore, may block

    auto result = std::async(std::launch::async,
                             [](auto&& f, auto&&... a) -> auto
                             {
                                 semaphore_guard callee_guard{async_limit,
                                                              guard_type::only_release};
                                 return std::invoke(std::forward<decltype(f)>(f),
                                                    std::forward<decltype(a)>(a)...);
                             },
                             std::forward<Func>(func),
                             std::forward<Args>(args)...);

    // If async() didn't fail, let the async thread handle the semaphore release.
    caller_guard.type = guard_type::only_acquire;

    return result;
}
