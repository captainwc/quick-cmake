#include <benchmark/benchmark.h>

static void foo(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(1.2 * 1.4);
    }
}

BENCHMARK(foo);

BENCHMARK_MAIN();
