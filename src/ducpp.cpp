#include <chrono>
#include <filesystem>
#include <future>
#include <queue>
#include <thread>
#include <vector>

#include "skutils/argparser.h"
#include "skutils/containers/topk_queue.h"
#include "skutils/macro.h"
#include "skutils/printer.h"
#include "skutils/threadpool.h"

namespace fs = std::filesystem;
namespace su = sk::utils;

bool validate(fs::path p) {
    if (!fs::exists(p)) {
        return false;
    }
    if (!(fs::is_regular_file(p) || fs::is_directory(p))) {
        return false;
    }
    auto perm = fs::status(p).permissions();
    return (fs::perms::owner_read & perm) != fs::perms::none;
}

struct CompFileSize {
    bool operator()(const fs::path &p1, const fs::path &p2) const {
        if (!validate(p1)) {
            return true;
        }
        if (!validate(p2)) {
            return false;
        }
        return fs::file_size(p1) < fs::file_size(p2);
    }
};

using QueueType = sk::utils::dts::topbottomk_queue<fs::path, CompFileSize>;

std::pair<double, std::string> format_size(const size_t sz) {
    if (sz < 1024) {
        return {sz, "B"};
    }
    if (sz < 1024 * 1024) {
        return {((double)sz / 1024.0), "KB"};
    }
    if (sz < 1024 * 1024 * 1024) {
        return {((double)sz / (1024.0 * 1024.0)), "MB"};
    }
    return {((double)sz / (1024.0 * 1024.0 * 1024.0)), "GB"};
}

std::pair<double, std::string> format_size(const fs::path &path) {
    try {
        return format_size(fs::file_size(path));
    } catch (const fs::filesystem_error &e) {
        SK_ERROR("Error: {}\n path1:{}\n path2:{}\n", e.what(), e.path1().generic_string(), e.path2().generic_string());
        return {0.1, "ERROR"};
    }
}

void topN(const fs::path &path, QueueType &top) {
    if (!validate(path)) {
        return;
    }
    if (fs::is_directory(path)) {
        for (const auto &e : fs::directory_iterator(path)) {
            topN(e, top);
        }
    } else {
        top.push(path);
    }
}

void topN_pool(const fs::path &path, QueueType &top) {
    sk::utils::ThreadPool pool;
    if (!validate(path)) {
        return;
    }
    if (fs::is_directory(path)) {
        std::vector<std::future<void>> fus;
        for (const auto &e : fs::directory_iterator(path)) {
            fus.emplace_back(pool.submit(topN, e, top));
        }
        for (auto &fu : fus) {
            fu.get();
        }
    } else {
        top.push(path);
    }
}

size_t du(const fs::path &path) {
    if (!validate(path)) {
        return 0;
    }
    size_t sz = 0;
    if (fs::is_directory(path)) {
        for (const auto &e : fs::directory_iterator(path)) {
            sz += du(e);
        }
    } else {
        sz += fs::file_size(path);
    }
    return sz;
}

size_t du_pool(const fs::path &path) {
    su::ThreadPool pool;
    if (!validate(path)) {
        return 0;
    }
    size_t sz = 0;
    if (fs::is_directory(path)) {
        std::vector<std::future<size_t>> fus;
        for (const auto &e : fs::directory_iterator(path)) {
            fus.emplace_back(pool.submit(du, e));
        }
        for (auto &fu : fus) {
            sz += fu.get();
        }
    } else {
        sz += fs::file_size(path);
    }
    return sz;
}

int main(int argc, char **argv) {
    su::arg::ArgParser parser;
    parser.add_arg({.name = "-p", .type = su::arg::ArgType::LIST, .help = "path"})
        .add_arg({.name = "-b", .type = su::arg::ArgType::INT, .help = "Find Smallest N File"})
        .add_arg({.name = "-t", .type = su::arg::ArgType::INT, .help = "Find Largest N File"});

    parser.parse(argc, argv);

    // int   argc1   = 7;
    // char *argv1[] = {"filename", "-p", "/home/shuaikai/test/quick-cmake", "-t", "3", "-b", "2"};
    // parser.parse(argc1, argv1);

    if (parser.need_help()) {
        parser.show_help();
        return 0;
    }

    auto paths = parser.get_value_with_default("-p").value_or(std::vector<std::string>{fs::current_path()});
    if (parser.get_value("-t").has_value() || parser.get_value("-b").has_value()) {
        int top_k    = std::get<int>(parser.get_value("-t").value_or(0));
        int bottom_k = std::get<int>(parser.get_value("-b").value_or(0));

        QueueType heap(std::max(top_k, bottom_k));

        for (const auto &p : paths) {
            topN(p, heap);
        }

        // std::vector<std::future<void>> fus;
        // for (const auto &p : paths) {
        //     fus.emplace_back(std::async(topN_pool, p, std::ref(heap)));
        // }
        // for (auto &fu : fus) {
        //     fu.get();
        // }

        if (top_k > 0) {
            auto top_k_files = heap.pop_top();
            top_k            = (top_k <= top_k_files.size() ? top_k : top_k_files.size());
            std::cout << sk::utils::colorful_format("Lagest {} Files in {}\n", top_k, paths);
            for (int i = 0; i < top_k; i++) {
                auto szp = format_size(top_k_files[i]);
                std::cout << sk::utils::colorful_format("[{} {}] <= {}\n", szp.first, szp.second,
                                                        top_k_files[i].generic_string());
            }
            std::cout << "\n";
        }

        if (bottom_k > 0) {
            auto bottom_k_files = heap.pop_bottom();
            bottom_k            = (bottom_k <= bottom_k_files.size() ? bottom_k : bottom_k_files.size());
            std::cout << sk::utils::colorful_format("Smallest {} Files in {}\n", bottom_k, paths);
            for (int i = 0; i < bottom_k; i++) {
                auto szp = format_size(bottom_k_files[i]);
                std::cout << sk::utils::colorful_format("[{} {}] <= {}\n", szp.first, szp.second,
                                                        bottom_k_files[i].generic_string());
            }
            std::cout << "\n";
        }

        return 0;
    }

    size_t sz = 0;

    std::vector<std::future<size_t>> fus;
    for (const auto &p : paths) {
        fus.emplace_back(std::async(du_pool, p));
    }
    for (auto &fu : fus) {
        sz += fu.get();
    }

    auto szp = format_size(sz);
    std::cout << sk::utils::colorful_format("[{} {}] {}\n", szp.first, szp.second, paths);

    return 0;
}
