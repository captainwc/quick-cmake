#ifndef COROUTINE_DEMO_CORO_AWAITABLE_HPP
#define COROUTINE_DEMO_CORO_AWAITABLE_HPP

// 仍然是类比多线程
// Task<T> 描述了协程的异步结果获取方式，对应多线程的 std::promise
// 那么Awaitable/Awaiter对应的就是多线程中的OS线程调度
// 进一步说，多线程时，你调用各种同步阻塞操作，OS会直接把你的线程挂起，然后执行别的线程
// 等你的线程期待的结果完成时，OS会唤醒当前线程，并从挂起的位置恢复执行，这些由OS完成
// 那么在协程中，你想获取异步任务结果，肯定不能再调用一个阻塞的操作让OS挂起了，因为OS眼里
// 只有线程，看不到上面跑了多少个协程。
// 所以需要用户去手动完成协程的挂起、恢复操作。这就是通过Awaitable/Awaiter来完成的
// 也即是说，Awaiter/Awaitable实际上对应的是OS的线程挂起和恢复。
// 协程中调用的异步IO，对于线程来说（我们的默认视角）它是异步的是非阻塞的。但是对于协程来说
// 一个异步IO就是阻塞了当前协程，然后用户调度别的协程上来执行了(线程就是那个cpu资源)
//
// 更进一步理解Awaiter和Awaitbale，需要搞清楚一个事实：
// co_await 实际上是一个单目操作符，它是操作符，是通过 operator co_await()重载的。
// 既然是操作符，那么就会有操作数。操作数是对象！而不是什么奇怪的函数调用方式
// co_await foo()，这里实际上是foo()构造了一个对象，它传给了co_await，然后拿到一个结果。
// 而不是co_await调用了foo这个函数。foo可以是一个构造函数，也可以是返回一个Awaitable的函数
//
// 搞明白了这点，就好理解Awaitable和Awaiter了：
// Awaitable 是co_await的操作数，它描述了一个具体的协程情景、任务，比如协程读文件
// Awaiter
// 是Awaitable被co_await操作后转换后的类型，它里面规定了协程应该如何挂起恢复，比如，应该异步读，读完后恢复协程处理读取的结果
// 而Task<T> 只是Awaitable这个协程情景协程任务的返回结果，比如它描述了异步读文件的结果（std::promise<read_result>
//
// Awaitbale可以通过co_await成员函数、ADL查找全局co_await函数转换为Awaiter，也可以自己就是Awaiter（包含三个函数）

#include <coroutine>
#include <string>

#include "async_oper.hpp"

// 控制如何异步读取文件
struct ReadFileAwaiter {};

// 描述一个异步任务
class ReadFileAwaitable {
private:
    std::string file_name_;
    std::string ret_;

public:
    ReadFileAwaitable(std::string file) : file_name_(file) {}

    // std::string operator co_await() {}

    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> handle) {
        // 读完了恢复协程
        ret_ = AsyncOper::AsyncRead(file_name_, [=]() { handle.resume(); });
    }

    std::string await_resume() { return ret_; }
};

#endif  // COROUTINE_DEMO_CORO_AWAITABLE_HPP
