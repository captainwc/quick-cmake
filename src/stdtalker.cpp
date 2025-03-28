#include <skutils/printer.h>
#include <skutils/threadpool.h>
#include <skutils/time_utils.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <future>
#include <thread>

using namespace sk::utils;

namespace {
std::atomic<bool> running{true};
std::atomic<int>  seq{1};
}  // namespace

void Worker(double sep) {
    static thread_local std::string start_time = time::current();
    static thread_local int         cnt        = 0;

    while (running.load()) {
        println("[{}] MSG {}", std::this_thread::get_id(), seq.fetch_add(1));
        ++cnt;

        // 将长时间睡眠分解为多次短时间睡眠，每次后检查running
        auto const start_sleep    = std::chrono::steady_clock::now();
        auto const total_sleep    = std::chrono::duration<double>(sep);
        auto       remaining_time = total_sleep;

        while (remaining_time > std::chrono::duration<double>::zero() && running.load()) {
            // 每次最多睡眠500ms，减少延迟
            auto const step = std::min(remaining_time, std::chrono::duration<double>(std::chrono::milliseconds(500)));
            std::this_thread::sleep_for(step);
            remaining_time = total_sleep - (std::chrono::steady_clock::now() - start_sleep);
        }
    }

    // println("[ID-{}][{} => {}]: {}", std::this_thread::get_id(), start_time, time::current(), cnt);
}

int main(int argc, char** argv) {
    double time_sep   = 1.0;
    int    thread_num = 1;

    if (argc > 3) {
        println("[Usage]: idle [time_sep] [thread_num]\n");
        return -1;
    }

    if (argc >= 2) {
        time_sep = std::atof(argv[1]);
    }

    if (argc == 3) {
        thread_num = std::atoi(argv[2]);  // 修正为std::atoi
    }

    ThreadPool pool;

    auto sig_handler = [](int sig) {
        if (sig == SIGINT) {
            running.store(false);
        }
    };

    std::signal(SIGINT, sig_handler);

    std::vector<std::future<void>> futures;
    for (int i = 0; i < thread_num; i++) {
        futures.emplace_back(pool.submit(Worker, time_sep));
    }

    for (auto& f : futures) {
        f.get();  // 等待所有线程完成
    }

    return 0;
}
