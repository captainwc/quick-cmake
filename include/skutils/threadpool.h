#ifndef SHUAIKAI_THREADPOOL_H
#define SHUAIKAI_THREADPOOL_H

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <optional>
#include <queue>
#include <type_traits>
#include <vector>

#include "noncopyable.h"

namespace sk::utils {

template <typename T>
class WorkQueue {
  private:
  std::queue<T> q_;
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

class ThreadPool : NonCopyable {
  private:
  using TaskType = std::function<void()>;

  std::mutex mtx_;
  std::condition_variable cv_;

  unsigned int corePoolSize_;
  std::atomic<bool> isRunning_;
  WorkQueue<TaskType> workQueue_;
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

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool(ThreadPool &&) = delete;

  ~ThreadPool() { shutdown(); }

  template <typename F, typename... Args>
  auto submit(F &&f, Args &&...args) {
    using RetType = std::invoke_result_t<F, Args...>;
    auto task = std::make_shared<std::packaged_task<RetType()>>(
      [func = std::forward<F>(f), ... captured_args = std::forward<Args>(args)]() mutable {
        return func(std::move(captured_args)...);
      });
    auto ret = task->get_future();
    workQueue_.push([task = std::move(task)]() { return (*task)(); });
    cv_.notify_one();
    return ret;
  }
};
}  // namespace sk::utils

#endif  // SHUAIKAI_THREADPOOL_H
