#ifndef SHUIAKIA_UTILS_FILE_H
#define SHUIAKIA_UTILS_FILE_H

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <future>
#include <ios>
#include <memory>
#include <string>
#include <system_error>

#include "config.h"
#include "logger.h"
#include "macro.h"
#include "noncopyable.h"

namespace fs = std::filesystem;

namespace sk::utils::file {

class FileInfo : public NonCopyable {
public:
    static std::string HomeDir() {
        if constexpr (IS_LINUX_OS()) {
            return std::getenv("HOME");
        } else {
            return std::getenv("USERPROFILE");
        }
    }

    explicit FileInfo(std::string filename) : file_path_(std::move(filename)) {}

    bool Exists() const { return fs::exists(file_path_); }

    size_t FileSize() const { return fs::file_size(file_path_); }

    bool Empty() const { return fs::is_empty(file_path_); }

    fs::path Parent() const { return file_path_.parent_path(); }

    fs::path FileName() const { return file_path_.filename(); }

    fs::path Extension() const { return file_path_.extension(); }

protected:
    fs::path file_path_;
};

class FileReader : public FileInfo {
private:
    std::ifstream in_;
    bool          valid_;

public:
    explicit FileReader(const std::string& filename) : FileInfo(filename) {
        if (Exists()) {
            in_    = (std::ifstream{file_path_, std::ios_base::in});
            valid_ = in_.is_open();
            if (!valid_) {
                SK_ERROR("Cannot Open File {}.", file_path_.string());
            }
        } else {
            valid_ = false;
            SK_ERROR("File UnExists: {}", filename);
        }
    }

    ~FileReader() {
        if (in_.is_open()) {
            in_.close();
        }
    }

    std::string ReadAll() {
        if (!valid_) {
            return "";
        }

        in_.seekg(0, std::ios::end);
        const auto size = in_.tellg();
        in_.seekg(0, std::ios::beg);

        std::string content;
        content.resize(size, '\0');

        if (!in_.read(content.data(), size)) {
            SK_ERROR("Read failed: {}", file_path_.string());
            return "";
        }

        return content;
    }

    std::string ReadLine() {
        if (!valid_) {
            return "";
        }
        std::string line;
        std::getline(in_, line);
        return line;
    }
};

}  // namespace sk::utils::file

#endif  // SHUIAKIA_UTILS_FILE_H
