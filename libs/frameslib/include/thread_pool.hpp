//
// Created by Alex Shchelochkov on 22.09.2022.
//
#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <vector>

namespace frameslib {
    class ThreadPool {
    protected:
        typedef std::function <void (void)> job_t;

        bool _shutdown;
        std::mutex _lock;
        std::condition_variable _cond_var;
        std::vector<std::thread> _threads;
        std::queue<job_t> _jobs;
        void thread_work(int id);
    public:
        explicit ThreadPool(int threads_cnt);
        ~ThreadPool();
        void add_job(job_t _job);
    };
}