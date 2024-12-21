#ifndef SHUAIKAI_THREADPOOL_H
#define SHUAIKAI_THREADPOOL_H

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

namespace sk::utils {

template <typename T>
class WorkQueue {
private:
    std::queue<T>      q_;
    mutable std::mutex mtx_;

public:
    [[nodiscard]] bool empty() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return q_.empty();
    }

    void push(T &&elem) {
        std::lock_guard<std::mutex> lock(mtx_);
        q_.push(std::move(elem));
    }

    void push(T &elem) {
        std::lock_guard<std::mutex> lock(mtx_);
        q_.push(elem);
    }

    std::optional<T> pop() {
        std::lock_guard<std::mutex> lock(mtx_);
        if (q_.empty()) {
            return std::nullopt;
        }
        auto ret = std::move(q_.front());
        q_.pop();
        return std::optional<T>(std::move(ret));
    }
};

class ThreadPool {
private:
    using TaskType = std::function<void()>;

    std::mutex              mtx_;
    std::condition_variable cv_;

    unsigned int             corePoolSize_;
    std::atomic<bool>        isRunning_;
    WorkQueue<TaskType>      workQueue_;
    std::vector<std::thread> workers_;

    void shutdown() {
        isRunning_.store(false);
        cv_.notify_all();
        for (auto &worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    void doWork() {
        while (isRunning_) {
            std::unique_lock<std::mutex> lock(mtx_);
            if (workQueue_.empty()) {
                cv_.wait(lock);
            }
            auto taskOpt = workQueue_.pop();
            lock.unlock();
            if (taskOpt.has_value()) {
                std::invoke(taskOpt.value());
            }
        }
    }

public:
    explicit ThreadPool(unsigned int corePoolSize = std::thread::hardware_concurrency())
        : corePoolSize_(corePoolSize), isRunning_(true) {
        for (int i = 0; i < corePoolSize_; ++i) {
            workers_.emplace_back(&ThreadPool::doWork, this);
        }
    }

    ThreadPool(const ThreadPool &)            = delete;
    ThreadPool(ThreadPool &&)                 = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool &operator=(ThreadPool &&)      = delete;

    ~ThreadPool() {
        shutdown();
    }

    template <typename F, typename... Args>
    auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))> {
        auto func     = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        auto task     = [task_ptr]() { (*task_ptr)(); };
        workQueue_.push(task);
        cv_.notify_one();
        return task_ptr->get_future();
    }
};
}  // namespace sk::utils

#endif  // SHUAIKAI_THREADPOOL_H
