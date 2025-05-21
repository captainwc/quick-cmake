#include <coroutine>

#include "coro_awaitable.hpp"
#include "coro_task.hpp"
#include "skutils/macro.h"

// 有了promise，有了挂起恢复（调度），但是还没有创建协程呢

// 这才是真正的协程（一个特殊的函数）
Task<std::string> async_read(std::string filename) {
    auto r = co_await ReadFileAwaitable(filename);
    SK_LOG("async_read coro finished! {}, {}", typeid(r).name(), r);
}

int main() {
    auto task1 = async_read("file1.txt");
    auto task2 = async_read("file2.txt");
    auto task3 = async_read("file3.txt");
    SK_LOG("GetResult from coro1! {}", task1.Get());
    SK_LOG("GetResult from coro2! {}", task2.Get());
    SK_LOG("GetResult from coro3! {}", task3.Get());
}