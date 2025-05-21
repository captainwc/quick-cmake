#ifndef COROUTINE_DEMO_CORO_TASK_HPP
#define COROUTINE_DEMO_CORO_TASK_HPP

#include <concepts>
#include <coroutine>
#include <exception>
#include <utility>

// 定义一个协程中的"std::promise"，它和具体的协程逻辑无关，只是描述结果
// 类比多线程，多线程中，用promise来描述不是立即返回的结果，而协程中对应的就是Task的概念。
// 看Task的要求中就有一个 promise_type 的要求，其实就说明了这一点
template <typename T = void>
class Task {
public:
    // 实际的promise_type，是强制要求
    struct promise_type;
    // 这个HandleType的类型也是固定的
    using HandleType = std::coroutine_handle<promise_type>;

    // 从 promise 中获取值，就像std::promise一样
    T Get() { return coro_handle_.promise().Get(); }

    // 五大件
    // TODO: 传参为什么要求是这玩意？
    Task(HandleType handle) : coro_handle_(handle) {}

    Task(const Task& task)            = delete;
    Task& operator=(const Task& task) = delete;

    Task(Task&& task) noexcept {
        // 移动构造时，先检查自己是否有需要销毁的句柄
        if (coro_handle_) {
            coro_handle_.destroy();
        }
        // 然后移动对方的句柄
        this->coro_handle_ = task.coro_handle_;
        // 将对方的句柄置空，避免重复销毁
        task.coro_handle_ = nullptr;
    }

    Task& operator=(Task&& task) noexcept {
        // 避免自我赋值
        if (this != &task) {
            // 先销毁自己的句柄
            if (coro_handle_) {
                coro_handle_.destroy();
            }
            // 然后移动对方的句柄
            this->coro_handle_ = task.coro_handle_;
            // 将对方的句柄置空，避免重复销毁
            task.coro_handle_ = nullptr;
        }
        return *this;
    }

    ~Task() {
        if (coro_handle_) {
            coro_handle_.destroy();
            // TODO: 还需要置空吗
            coro_handle_ = nullptr;
        }
    }

private:
    // 实际上是一个指针
    HandleType coro_handle_;
};

/// MARK:  promise_type的具体要求
template <typename T>
struct Task<T>::promise_type {
    // 调用过程中的异常
    std::exception_ptr exp_ptr;
    // 这个Task(也即promise)保存的值，也即调用者真正想得到的值
    T promised_value;

    T Get() {
        if (exp_ptr) {
            std::rethrow_exception(exp_ptr);
        }
        return promised_value;
    }

    /// MARK: 强制要求的函数
    Task get_return_object() { return Task{HandleType::from_promise(*this)}; }

    // 协程是一个特殊的函数，这一句控制调用协程（函数）的时候，是否立即执行函数（的第一句）
    auto initial_suspend() {
        // 永不立即执行
        return std::suspend_never{};
    }

    // 协程的生命周期控制。是最后一次挂起协程的机会，也即协程执行到函数最后一句话了，再问一次
    // 还有人要挂起我么？如果没有，则自动负责资源回收。如果有，则用户负责析构资源
    // 使用 std::suspend_always 确保协程在完成时挂起，而不是自动销毁
    // 这样可以让 Task 对象控制协程句柄的生命周期，避免段错误
    auto final_suspend() noexcept { return std::suspend_always{}; }

    void unhandled_exception() { exp_ptr = std::current_exception(); }

    // return_value 和 return_void 二选一，他们是co_return的返回值，也即协程任务的promise值
    // 注意co_await的返回值是await_retume提供的。
    // 区分一下，协程函数调用者关心的是promise值（由co_return提供）；
    // 而协程内部实现时才用到co_await的值（获取异步操作结果等等）
    // void return_void() {}
    void return_value(T val) { promised_value = val; }

    // 可选
    template <std::convertible_to<T> From>
    auto yield_value(From&& from) {
        promised_value = std::forward<From>(from);
        return std::suspend_always{};  // yield每调用一次都要暂停一下
    }
};

#endif  // COROUTINE_DEMO_CORO_TASK_HPP
