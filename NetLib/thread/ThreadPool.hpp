//
// Created by Cmf on 2022/5/30.
//

#ifndef CMFNETLIB_THREADPOOL_HPP
#define CMFNETLIB_THREADPOOL_HPP

#include <deque>
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <future>
#include <functional>
#include <condition_variable>

class ThreadPool {
public:
    using ptr = std::shared_ptr<void()>;

public:

    explicit ThreadPool(size_t taskSize, size_t threadSize = std::thread::hardware_concurrency()) :
            _tasks(0),
            _maxTaskSize(taskSize),
            _running(false) {

        if (!_threads.empty())return;
        _running = true;
        _threads.reserve(threadSize);
        for (size_t i = 0; i < threadSize; ++i) {
            _threads.emplace_back([this]() {
                while (true) {
                    Task task;
                    {
                        std::unique_lock<std::mutex> lk(_mtx);
                        _not_empty.wait(lk, [this] {
                            return !_running.load() || !_tasks.empty();
                        });
                        if (!_running.load() && _tasks.empty())return;
                        task = std::move(_tasks.front());
                        _tasks.pop_front();
                    }
                    _not_full.notify_one();
                    task();
                }
            });
        }
    }

    ~ThreadPool() noexcept {
        _running = false;
        _not_empty.notify_all();
        _not_full.notify_all();
        for (auto &thr: _threads) {
            if (thr.joinable()) {
                thr.join();
            }
        }
    }

    bool Full() const {
        return _maxTaskSize > 0 && _tasks.size() >= _maxTaskSize;
    }

    template<typename F, typename... Args>
    auto Commit(F &&f, Args &&...args) -> std::future<decltype(f(args...))> {
        if (!_running.load()) {
            throw std::runtime_error("Commit on ThreadPool is Stopped.");
        }
        using RetType = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<RetType()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)); // 把函数入口及参数,打包(绑定)
        std::future<RetType> future = task->get_future();
        {
            std::unique_lock<std::mutex> lk(_mtx);
            _not_full.wait(lk, [this] {
                return !_running.load() || !Full();
            });
            _tasks.emplace_back([task] {
                (*task)();
            });
        }
        _not_empty.notify_one();
        return future;
    }

private:
    using Task = std::function<void()>;

private:
    std::atomic<size_t> _maxTaskSize;
    std::deque<Task> _tasks;
    std::vector<std::thread> _threads;
    std::condition_variable _not_empty;
    std::condition_variable _not_full;
    std::mutex _mtx;
    std::atomic<bool> _running;
};

#endif //CMFNETLIB_THREADPOOL_HPP
