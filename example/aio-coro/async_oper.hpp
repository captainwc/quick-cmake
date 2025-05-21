#ifndef COROUTINE_DEMO_ASYNC_OPER_HPP
#define COROUTINE_DEMO_ASYNC_OPER_HPP

#include <skutils/macro.h>

#include <chrono>
#include <future>
#include <thread>

using namespace std::literals;

/// MARK: (用多线程来模拟)真正的异步IO
class AsyncOper {
public:
    template <typename CallBack_t>
    static void AsyncSleep(int time, CallBack_t callback, std::string _msg = "Sleeping") {
        std::async(std::launch::async, [=]() {
            SK_LOG("Async {}...", _msg);
            std::this_thread::sleep_for(std::chrono::milliseconds(time));
            // 模拟异步操作的通知方式(这里是通过coro_handle恢复协程)
            std::invoke(callback);
        });
    }

    template <typename CallBack_t>
    static std::string AsyncRead(std::string filename, CallBack_t callback) {
        AsyncSleep(4000, callback, std::format("Reading {}", filename));
        return std::format("Read result from {}", filename);
    }

    template <typename CallBack_t>
    static size_t AsyncWrite(std::string filename, std::string content, CallBack_t callback) {
        AsyncSleep(6000, callback, std::format("Writing {} to {}", content, filename));
        return 1234;
    }
};

#endif  // COROUTINE_DEMO_ASYNC_OPER_HPP
