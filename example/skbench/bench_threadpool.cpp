#include <benchmark/benchmark.h>

#include <filesystem>
#include <fstream>
#include <vector>

#include "skutils/macro.h"
#include "skutils/threadpool.h"

namespace fs = std::filesystem;

const int scacle = 16;

int read_file() {
    fs::path file(fs::current_path() / ".." / ".." / "example" / "sktest" / "sk_benchmark_threadpool.cpp");
    int      bytes = 0;
    if (fs::exists(file)) {
        std::ifstream rdfile(file);
        if (rdfile.is_open()) {
            std::string line;
            int         i = 0;
            while (std::getline(rdfile, line)) {
                bytes += line.length();
            }
            return bytes;
        }
        return -1;
    }
    SK_ERROR("\"{}\" doesn't exist!", file.string());
    return -1;
}

auto task() {
    return read_file();
}

static void BM_POOL(benchmark::State& state) {
    sk::utils::ThreadPool pool;
    int                   bytes = 0;
    for (auto _ : state) {
        std::vector<std::future<int>> fs;
        fs.reserve(scacle);
        for (int i = 0; i < scacle; i++) {
            fs.emplace_back(pool.submit(task));
        }
        for (auto& f : fs) {
            benchmark::DoNotOptimize(bytes = f.get());
        }
    }
    SK_LOG("POOL Read {} bytes from {}", bytes, "BM_ThreadPool.cpp");
}

static void BM_NOPOOL(benchmark::State& state) {
    int bytes = 0;
    for (auto _ : state) {
        for (int i = 0; i < scacle; i++) {
            benchmark::DoNotOptimize(bytes = task());
        }
    }
    SK_LOG("NO_POOL Read {} bytes from {}", bytes, "BM_ThreadPool.cpp");
}

BENCHMARK(BM_POOL);
BENCHMARK(BM_NOPOOL);

BENCHMARK_MAIN();
