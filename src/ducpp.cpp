#include <algorithm>
#include <filesystem>
#include <functional>
#include <future>
#include <string>
#include <vector>

#include "skutils/argparser.h"
#include "skutils/containers/topk_queue.h"
#include "skutils/macro.h"
#include "skutils/printer.h"
#include "skutils/threadpool.h"

namespace fs = std::filesystem;
namespace su = sk::utils;

inline bool validate(const fs::path &p) {
    return fs::exists(p);
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

void topN(const fs::path &path, QueueType &top, const std::function<bool(const fs::path &)> &filter) {
    if (!validate(path)) {
        return;
    }
    if (fs::is_directory(path)) {
        for (const auto &e : fs::directory_iterator(path)) {
            topN(e, top, filter);
        }
    } else {
        if (filter(path)) {
            top.push(path);
        }
    }
}

void topN_pool(const fs::path &path, QueueType &top, const std::function<bool(const fs::path &)> &filter) {
    sk::utils::ThreadPool pool;
    if (!validate(path)) {
        return;
    }
    if (fs::is_directory(path)) {
        std::vector<std::future<void>> fus;
        for (const auto &e : fs::directory_iterator(path)) {
            fus.emplace_back(pool.submit(topN, e, std::ref(top), filter));
        }
        for (auto &fu : fus) {
            fu.get();
        }
    } else {
        if (filter(path)) {
            top.push(path);
        }
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
        .add_arg({.name = "-t", .type = su::arg::ArgType::INT, .help = "Find Largest N File"})
        .add_arg({.name = "-e",
                  .type = su::arg::ArgType::LIST,
                  .help = "File Extention to find largest/smallest, like md, cpp etc. Only used when -t or -b is set"});

    parser.parse(argc, argv);

    // int   argc1   = 7;
    // char *argv1[] = {"filename", "-p", "/home/shuaikai/", "-t", "3", "-b", "2"};
    // parser.parse(argc1, argv1);

    if (parser.need_help()) {
        parser.show_help();
        return 0;
    }

    auto paths = parser.get_value_with_default("-p").value_or(std::vector<std::string>{fs::current_path().generic_string()});

    if (parser.get_value("-t").has_value() || parser.get_value("-b").has_value()) {
        std::vector<std::string> extentions =
            std::get<std::vector<std::string>>(parser.get_value("-e").value_or(std::vector<std::string>{}));

        std::for_each(extentions.begin(), extentions.end(), [](std::string &ext) {
            if (ext[0] != '.') {
                ext = "." + ext;
            }
        });

        std::function<bool(const fs::path &p)> filter = [&extentions](const fs::path &p) -> bool {
            if (extentions.empty()) {
                return true;
            }
            if (std::find(extentions.begin(), extentions.end(), p.extension().generic_string()) != extentions.end()) {
                return true;
            }
            return false;
        };

        int top_k    = std::get<int>(parser.get_value("-t").value_or(0));
        int bottom_k = std::get<int>(parser.get_value("-b").value_or(0));

        QueueType heap(std::max(top_k, bottom_k));

        for (const auto &p : paths) {
            topN_pool(p, heap, std::ref(filter));
        }

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
