#include <benchmark/benchmark.h>

#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_set>

constexpr int kSetSize = 100;

class MyBenchmark : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override {
        std::call_once(flag, [this]() {
            s = std::make_shared<std::unordered_set<int>>();
            for (int i = 0; i < kSetSize; i++) {
                s->insert(i);
            }
        });
    }

    /**
     * ! 缓存乒乓 （Cache Ping Pong)
     * * 使用Shared_Ptr，其中有引用计数，则多个处理器会同时竞争这个cache行
     */
    std::shared_ptr<std::unordered_set<int>> GetSet() { return s; }

private:
    std::shared_ptr<std::unordered_set<int>> s;
    std::once_flag                           flag;
};

class MyBenchmark2 : public benchmark::Fixture {
public:
    void SetUp(const ::benchmark::State& state) override {
        std::call_once(flag, [this]() {
            for (int i = 0; i < kSetSize; i++) {
                s.insert(i);
            }
        });
    }

    /**
     * * 去掉 shared_ptr
     */
    const std::unordered_set<int>& GetSet() { return s; }

private:
    std::unordered_set<int> s;
    std::once_flag          flag;
};

BENCHMARK_DEFINE_F(MyBenchmark, MultiThreadedWork)(benchmark::State& state) {
    for (auto _ : state) {
        int size_sum        = kSetSize * 2;
        int size_per_thread = (size_sum + state.threads() - 1) / state.threads();
        int sum             = 0;
        int start           = state.thread_index() * size_per_thread;
        int end             = std::min((state.thread_index() + 1) * size_per_thread, size_sum);
        for (int i = start; i < end; i++) {
            auto inst = GetSet();
            if (inst->count(i) > 0) {
                sum++;
            }
        }
    }
}

BENCHMARK_DEFINE_F(MyBenchmark2, MultiThreadedWork)(benchmark::State& state) {
    for (auto _ : state) {
        int size_sum        = kSetSize * 2;
        int size_per_thread = (size_sum + state.threads() - 1) / state.threads();
        int sum             = 0;
        int start           = state.thread_index() * size_per_thread;
        int end             = std::min((state.thread_index() + 1) * size_per_thread, size_sum);
        for (int i = start; i < end; i++) {
            const auto& inst = GetSet();
            if (inst.count(i) > 0) {
                benchmark::DoNotOptimize(sum++);
            }
        }
    }
}

// 注册基准测试，并指定线程数
BENCHMARK_REGISTER_F(MyBenchmark, MultiThreadedWork)->Threads(1);
BENCHMARK_REGISTER_F(MyBenchmark, MultiThreadedWork)->Threads(2);
BENCHMARK_REGISTER_F(MyBenchmark, MultiThreadedWork)->Threads(4);
BENCHMARK_REGISTER_F(MyBenchmark, MultiThreadedWork)->Threads(8);

BENCHMARK_REGISTER_F(MyBenchmark2, MultiThreadedWork)->Threads(1);
BENCHMARK_REGISTER_F(MyBenchmark2, MultiThreadedWork)->Threads(2);
BENCHMARK_REGISTER_F(MyBenchmark2, MultiThreadedWork)->Threads(4);
BENCHMARK_REGISTER_F(MyBenchmark2, MultiThreadedWork)->Threads(8);

BENCHMARK_MAIN();