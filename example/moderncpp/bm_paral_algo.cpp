#include <benchmark/benchmark.h>

#include <execution>
#include <numeric>
#include <vector>

#include "skutils/random.h"

/**
 * We can conlude that, when data size less than 70000, paral algorithm is no better than the trivial one.
 */

static std::vector<int> data{RANDTOOL.getRandomIntVector(70000)};

static void bm_reduce(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(std::reduce(data.begin(), data.end()));
  }
}

static void bm_reduce_par(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(std::reduce(std::execution::par, data.begin(), data.end()));
  }
}

BENCHMARK(bm_reduce);
BENCHMARK(bm_reduce_par);

BENCHMARK_MAIN();
