//
// Created by Alex Shchelochkov on 25.07.2022.
//

#include "../include/thread_pool.hpp"

using namespace std;

mllib::ThreadPool::ThreadPool(int threads_cnt) : _shutdown(false) {
    _threads.reserve(threads_cnt);
    for (int i = 0; i < threads_cnt; i++)
        _threads.emplace_back([this, i] { thread_work(i); });
}

mllib::ThreadPool::~ThreadPool() {
    {
        unique_lock<mutex> _unique_lock(_lock);
        _shutdown = true;
        _cond_var.notify_all();
    }
    for (auto &thread : _threads)
        thread.join();
}

void mllib::ThreadPool::thread_work(int id) {
    job _job;
    while(true) {
        {
            unique_lock<mutex> _unique_lock(_lock);
            while (!_shutdown && _jobs.empty())
                _cond_var.wait(_unique_lock);
            if (_jobs.empty())
                // Thread id was terminated.
                return;
            _job = move(_jobs.front());
            _jobs.pop();
        }
        _job();
    }
}

void mllib::ThreadPool::add_job(job _job) {
    unique_lock<mutex> _unique_lock(_lock);
    _jobs.emplace(_job);
    _cond_var.notify_one();
}