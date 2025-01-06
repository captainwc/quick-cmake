#include <benchmark/benchmark.h>

#include "skutils/threadpool.h"

using namespace sk::utils;

// 创建一个较大的vector并对其进行排序作为模拟重任务
void HeavyTask(int n) {
    std::vector<int> vec(n);
    for (int i = 0; i < n; ++i) {
        vec[i] = rand() % 1000;
    }
    std::sort(vec.begin(), vec.end());
}

// 基准测试函数
static void BM_ThreadPool(benchmark::State& state) {
    ThreadPool pool(state.range(0));  // 使用 参数1 指定线程池大小

    // 每次迭代中要提交的任务数量 参数2
    const int task_count = state.range(1);

    for (auto _ : state) {
        state.PauseTiming();  // 暂停计时，避免初始化时间的影响
        std::vector<std::future<void>> futures;
        futures.reserve(task_count);

        // 提交所有任务
        for (int i = 0; i < task_count; ++i) {
            futures.emplace_back(pool.submit(&HeavyTask, 1000));
        }

        state.ResumeTiming();  // 恢复计时
        // 等待所有任务完成
        for (auto& fut : futures) {
            fut.get();
        }
    }
}

// 注册基准测试函数，并设置线程池大小和任务数量作为参数
BENCHMARK(BM_ThreadPool)
    ->Args({4, 500})  // 4个线程，500个任务
    ->Args({8, 500})
    ->Args({16, 500})
    ->Args({20, 500})
    ->Args({24, 500})
    ->Args({32, 500})
    ->Args({36, 500})
    ->Args({40, 500})
    ->Args({44, 500})
    ->Args({48, 500})
    ->UseRealTime();  // 使用真实时间而非CPU时间

BENCHMARK_MAIN();  // 主函数入口点