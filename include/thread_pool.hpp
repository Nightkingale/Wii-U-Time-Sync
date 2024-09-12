/*
 * Wii U Time Sync - A NTP client plugin for the Wii U.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <atomic>
#include <exception>
#include <functional>           // bind(), move_only_function<>
#include <future>
#include <mutex>
#include <thread>
#include <type_traits>          // decay_t<>, invoke_result_t<>
#include <utility>              // forward(), move()
#include <vector>

#include "async_queue.hpp"


class thread_pool {

    unsigned max_workers;

    std::vector<std::jthread> workers;

    // Note: we can't use std::function because we're putting std::packaged_task in there,
    // and std::packaged_task is only movable, but std::function always tries to copy.
    using task_type = std::move_only_function<void()>;
    async_queue<task_type> tasks;

    std::atomic_int num_idle_workers = 0;

    void worker_thread(std::stop_token token);

    void add_worker();

public:

    thread_pool(unsigned max_workers);

    ~thread_pool();


    // This method behaves like std::async().
    template<typename Func, typename... Args>
    std::future<std::invoke_result_t<std::decay_t<Func>,
                                     std::decay_t<Args>...>>
    submit(Func&& func, Args&&... args)
    {
        using Ret = std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>;

        auto bfunc = std::bind(std::forward<Func>(func),
                               std::forward<Args>(args)...);

        std::packaged_task<Ret()> task{std::move(bfunc)};
        auto future = task.get_future();

        if (max_workers == 0)
            task(); // If no worker will handle this, execute it immediately.
        else {
            // If all threads are busy, try to add another to the pool.
            if (num_idle_workers == 0)
                add_worker();

            tasks.push(std::move(task));
        }

        return future;
    }

};

#endif
