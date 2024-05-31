// SPDX-License-Identifier: MIT

#include "thread_pool.hpp"


void
thread_pool::worker_thread(std::stop_token token)
{
    try {
        while (!token.stop_requested()) {
            auto task = tasks.pop();
            --num_idle_workers;
            task();
            ++num_idle_workers;
        }
    }
    catch (async_queue<task_type>::stop_request& r) {}
}


void
thread_pool::add_worker()
{
    // Obey the limit.
    if (workers.size() >= max_workers)
        return;
    ++num_idle_workers;
    workers.emplace_back([this](std::stop_token token) { worker_thread(token); });
}


thread_pool::thread_pool(unsigned max_workers) :
    max_workers{max_workers}
{}


thread_pool::~thread_pool()
{
    // This will wake up all threads stuck waiting for more tasks, they will all throw
    // tasks_queue::stop_request{}.
    tasks.stop();

    // The jthread destructor will also notify the stop token, so we don't need to do
    // anything else.
}
