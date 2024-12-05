#include <benchmark/benchmark.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <vector>

#include "skutils/threadpool.h"

namespace fs = std::filesystem;

const auto logger = spdlog::basic_logger_mt("BM_ThreadPool", "log/BM_ThreadPool.log");

const int scacle = 16;

int read_file() {
    fs::path file("../../benchmark/BM_ThreadPool.cpp");
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
    SPDLOG_LOGGER_CRITICAL(logger, "\"{}\" doesn't exist!", file.string());
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
    SPDLOG_LOGGER_CRITICAL(logger, "POOL Read {} bytes from {}", bytes, "BM_ThreadPool.cpp");
}

static void BM_NOPOOL(benchmark::State& state) {
    int bytes = 0;
    for (auto _ : state) {
        for (int i = 0; i < scacle; i++) {
            benchmark::DoNotOptimize(bytes = task());
        }
    }
    SPDLOG_LOGGER_CRITICAL(logger, "NO_POOL Read {} bytes from {}", bytes, "BM_ThreadPool.cpp");
}

BENCHMARK(BM_POOL);
BENCHMARK(BM_NOPOOL);

BENCHMARK_MAIN();