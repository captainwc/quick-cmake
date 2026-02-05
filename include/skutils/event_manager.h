#pragma once

#ifndef SK_UTILS_EVENT_MANAGER_H
#define SK_UTILS_EVENT_MANAGER_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#include "typeinfo.h"

namespace sk::utils {
class EventBus {
public:
    static EventBus& GetInstance() {
        static EventBus instance;
        return instance;
    }

    EventBus(const EventBus&)            = delete;
    EventBus& operator=(const EventBus&) = delete;

    template <typename T>
    void Subscribe(std::function<void(const T&)> callback) {
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto                        typeIdx = type::GetTypeID<T>();

        if (subscribers_.find(typeIdx) == subscribers_.end()) {
            subscribers_[typeIdx] = std::make_shared<HandlerList<T>>();
        }

        auto handler = std::static_pointer_cast<HandlerList<T>>(subscribers_[typeIdx]);
        handler->Add(callback);
    }

    // 同步发布：在当前线程立即执行所有回调
    template <typename T>
    void Publish(const T& event) {
        auto handlers = GetHandlers<T>();
        if (handlers) {
            for (const auto& cb : handlers->callbacks) {
                cb(event);
            }
        }
    }

    // 异步发布：放入线程池执行
    template <typename T>
    void PublishAsync(const T& event) {
        workerQueue_.Push([this, event]() { this->Publish(event); });
    }

    // 延迟发布 (定时/超时)：在指定时间后发布
    template <typename T>
    void PublishDelayed(const T& event, uint32_t delayMs) {
        auto runTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(delayMs);

        std::lock_guard<std::mutex> lock(timerMutex_);
        timerQueue_.push({runTime, [this, event]() {
                              this->PublishAsync(event);  // 时间到了之后，转入异步队列执行
                          }});
        timerCv_.notify_one();
    }

    ~EventBus() { Stop(); }

private:
    EventBus() : stop_(false) {
        for (int i = 0; i < 4; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    auto task = workerQueue_.Pop();
                    if (!task)
                        break;  // 停止信号
                    (*task)();
                }
            });
        }

        timerThread_ = std::thread([this] {
            while (!stop_) {
                std::unique_lock<std::mutex> lock(timerMutex_);

                if (timerQueue_.empty()) {
                    timerCv_.wait(lock);
                } else {
                    auto  now = std::chrono::steady_clock::now();
                    auto& top = timerQueue_.top();

                    if (now >= top.timePoint) {
                        // 时间到了，取出任务并执行
                        auto task = top.task;
                        timerQueue_.pop();
                        lock.unlock();  // 解锁以允许在任务入队时避免死锁
                        task();
                    } else {
                        // 时间没到，继续等待
                        timerCv_.wait_until(lock, top.timePoint);
                    }
                }
            }
        });
    }

    void Stop() {
        if (stop_.exchange(true))
            return;

        workerQueue_.Stop();
        for (auto& t : workers_) {
            if (t.joinable())
                t.join();
        }

        timerCv_.notify_all();
        if (timerThread_.joinable())
            timerThread_.join();
    }

    struct HandlerBase {
        virtual ~HandlerBase() = default;
    };

    template <typename T>
    struct HandlerList : public HandlerBase {
        std::vector<std::function<void(const T&)>> callbacks;

        void Add(std::function<void(const T&)> cb) { callbacks.push_back(cb); }
    };

    template <typename T>
    std::shared_ptr<HandlerList<T>> GetHandlers() {
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto                        typeIdx = type::GetTypeID<T>();
        auto                        it      = subscribers_.find(typeIdx);
        if (it != subscribers_.end()) {
            return std::static_pointer_cast<HandlerList<T>>(it->second);
        }
        return nullptr;
    }

    struct TaskQueue {
        std::queue<std::function<void()>> queue;
        std::mutex                        mtx;
        std::condition_variable           cv;
        bool                              stopped = false;

        void Push(std::function<void()> task) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                if (stopped)
                    return;
                queue.push(std::move(task));
            }
            cv.notify_one();
        }

        std::optional<std::function<void()>> Pop() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return !queue.empty() || stopped; });
            if (queue.empty() && stopped)
                return std::nullopt;
            auto task = std::move(queue.front());
            queue.pop();
            return task;
        }

        void Stop() {
            {
                std::lock_guard<std::mutex> lock(mtx);
                stopped = true;
            }
            cv.notify_all();
        }
    } workerQueue_;

    struct TimerTask {
        std::chrono::steady_clock::time_point timePoint;
        std::function<void()>                 task;

        bool operator<(const TimerTask& other) const { return timePoint > other.timePoint; }
    };

    std::unordered_map<type::TypeID, std::shared_ptr<HandlerBase>> subscribers_;
    std::mutex                                                     mapMutex_;

    std::vector<std::thread> workers_;

    std::priority_queue<TimerTask> timerQueue_;
    std::mutex                     timerMutex_;
    std::condition_variable        timerCv_;
    std::thread                    timerThread_;

    std::atomic<bool> stop_;
};

}  // namespace sk::utils

#endif  // SK_UTILS_EVENT_MANAGER_H
