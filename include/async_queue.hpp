/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ASYNC_QUEUE_HPP
#define ASYNC_QUEUE_HPP

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>              // forward(), move()


template<typename T,
         typename Q = std::queue<T>>
class async_queue {

    std::mutex mutex;
    std::condition_variable empty_cond;
    Q queue;
    bool should_stop = false;

public:

    struct stop_request {};

    // Makes the pool usable again after a stop().
    void
    reset()
    {
        std::lock_guard guard{mutex};
        should_stop = false;
    }


    // This will make all future pop() calls throw a stop_request{}.
    // It also wakes up all threads waiting on empty_cond.
    void
    stop()
    {
        std::lock_guard guard{mutex};
        should_stop = true;
        empty_cond.notify_all(); // make sure all threads can see the updated flag
    }


    bool
    is_stopping() const
    {
        std::lock_guard guard{mutex};
        return should_stop;
    }


    bool
    empty()
        const
    {
        std::lock_guard guard{mutex};
        return queue.empty();
    }


    template<typename U>
    void
    push(U&& x)
    {
        std::lock_guard guard{mutex};
        queue.push(std::forward<U>(x));
        empty_cond.notify_one();
    }


    T
    pop()
    {
        std::unique_lock guard{mutex};
        // Stop waiting if a stop was requested, or the queue as data.
        empty_cond.wait(guard, [this] { return should_stop || !queue.empty(); });
        if (should_stop)
            throw stop_request{};
        T result = std::move(queue.front());
        queue.pop();
        return result;
    }


    template<typename U>
    bool
    try_push(U&& x)
    {
        std::unique_lock guard{mutex, std::try_to_lock};
        if (!guard)
            return false;
        queue.push(std::forward<U>(x));
        return true;
    }


    std::optional<T>
    try_pop()
    {
        std::unique_lock guard{mutex, std::try_to_lock};
        if (!guard)
            return {};
        if (queue.empty())
            return {};
        if (should_stop)
            throw stop_request{};
        T result = std::move(queue.front());
        queue.pop();
        return result;
    }

};


#endif
