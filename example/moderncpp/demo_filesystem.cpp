#include <filesystem>

#include "skutils/macro.h"
#include "skutils/printer.h"

namespace fs = std::filesystem;

void list_dir(const fs::path& dir) {
    if (!fs::exists(dir)) {
        SK_ERROR("Directory {} unexisis.", dir);
        return;
    }
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (fs::is_directory(entry)) {
            // list_dir(entry);
            SK_WARN("MET DIRECTORY {}", fs::absolute(entry));
        } else {
            SK_LOG("NAME:{}, SIZE:{} B", fs::absolute(entry).filename(), entry.file_size());
        }
    }
}

struct A {};

int main() {
    DUMP(fs::current_path());

    fs::path target_dir("../");
    list_dir(target_dir);
}