#include <cstddef>
#include <filesystem>
#include <optional>
#include <vector>

#include "skutils/macro.h"
#include "skutils/printer.h"

namespace fs = std::filesystem;

template <typename Cond>
auto walk_dir(const fs::path& dir, const Cond& pred) -> std::optional<std::vector<fs::path>> {
    if (!fs::exists(dir)) {
        SK_ERROR("Directory {} unexisis.", dir);
        return std::nullopt;
    }
    std::vector<fs::path> ret;
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (fs::is_directory(entry)) {
            walk_dir(entry, pred);
        } else {
            if (pred(entry)) {
                ret.emplace_back(entry);
            }
        }
    }
    return ret;
}

struct A {};

int main() {
    DUMP(fs::current_path());

    fs::path target_dir("/mnt/c/Users/wddjwk/Pictures/Saved Pictures/");
    auto     ret_opt = walk_dir(target_dir, [](const fs::path& p) {
        return fs::file_size(p) < static_cast<unsigned long>(5 * 1024 * 1024);
    });

    if (ret_opt.has_value()) {
        for (const auto& p : ret_opt.value()) {
            std::cout << p << std::endl;
        }
        SK_LOG("Pic Num: {}", ret_opt.value().size());
    }
}
