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
        std::this_thread::sleep_for(std::chrono::duration<double>(sep));
    }

    println("[ID-{}][{} => {}]: {}", std::this_thread::get_id(), start_time, time::current(), cnt);
}

int main(int argc, char** argv) {
    double time_sep   = 1.0;
    int    thread_num = 1;

    if (argc > 3) {
        println("[Usage]: idle [time_sep] [thread_num]\n");
        return -1;
    }

    if (argc == 2) {
        time_sep = std::atof(argv[1]);
    }

    if (argc == 3) {
        time_sep   = std::atof(argv[1]);
        thread_num = std::atof(argv[2]);
    }

    auto sig_handler = [](int sig) {
        if (sig == SIGINT) {
            running.store(false);
        }
    };

    std::signal(SIGINT, sig_handler);

    ThreadPool pool;

    std::vector<std::future<void>> futures;
    for (int i = 0; i < thread_num; i++) {
        futures.emplace_back(pool.submit(Worker, time_sep));
    }

    for (auto& f : futures) {
        f.get();
    }

    return 0;
}
