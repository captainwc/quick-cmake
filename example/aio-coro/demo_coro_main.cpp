#include <vector>

#include "coro_awaitable.hpp"
#include "coro_task.hpp"
#include "skutils/logger.h"
#include "skutils/test.h"

// 有了promise，有了挂起恢复（调度），但是还没有创建协程呢

// 这才是真正的协程（一个特殊的函数，必须返回一个包含promise_type的类型）
Task<std::string> async_read(std::vector<std::string> files) {
    // co_await 挂起当前协程，等待异步操作完成。结果是从await_resume()中获取的
    std::string ret;
    for (const auto& filename : files) {
        auto r = co_await ReadFileAwaitable(filename);
        SK_LOG("async_read coro finished! {}, {}", typeid(r).name(), r);
        ret.append(r).append("\n");
    }
    // 使用co_return返回结果给调用者，会将值存储在promise的promised_value中
    co_return ret;
}

void start_coro() {
    auto task1 = async_read({"file1.txt", "file2.txt", "file3.txt"});
    // 创建协程之后，必须要启动才会开始执行
    // 另外，如果协程initial_suspend设置为always，还是需要先恢复才会执行
    SK_LOG("GetResult from coro1! {}", task1.Get());
}

int main() {
    RUN_DEMO(start_coro);
}
