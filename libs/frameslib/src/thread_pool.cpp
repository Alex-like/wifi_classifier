//
// Created by Alex Shchelochkov on 22.09.2022.
//

#include "../include/thread_pool.hpp"

frameslib::ThreadPool::ThreadPool(int threads_cnt) : _shutdown(false) {
    _threads.reserve(threads_cnt);
    for (int i = 0; i < threads_cnt; i++)
        _threads.emplace_back([this, i] { thread_work(i); });
}

frameslib::ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> _unique_lock(_lock);
        _shutdown = true;
        _cond_var.notify_all();
    }
    for (auto &thread : _threads)
        thread.join();
}

void frameslib::ThreadPool::thread_work(int id) {
    job_t _job;
    while(true) {
        {
            std::unique_lock<std::mutex> _unique_lock(_lock);
            while (!_shutdown && _jobs.empty())
                _cond_var.wait(_unique_lock);
            if (_jobs.empty())
                // Thread id was terminated.
                return;
            _job = std::move(_jobs.front());
            _jobs.pop();
        }
        _job();
    }
}

void frameslib::ThreadPool::add_job(job_t _job) {
    std::unique_lock<std::mutex> _unique_lock(_lock);
    _jobs.emplace(_job);
    _cond_var.notify_one();
}