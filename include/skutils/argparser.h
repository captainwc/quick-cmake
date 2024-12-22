#ifndef SHUAIKAI_UTILS_ARGS_PARSER_H
#define SHUAIKAI_UTILS_ARGS_PARSER_H

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include "skutils/macro.h"
#include "skutils/printer.h"
#include "skutils/string_utils.h"

namespace sk::utils::arg {

using ArgValueType = std::variant<bool, int, double, std::string, std::vector<std::string>>;

template <typename T, typename = void>
struct ArgConcept : std::false_type {};

template <typename T>
struct ArgConcept<T,
                  std::enable_if_t<std::is_same_v<int, T> || std::is_same_v<double, T> || std::is_same_v<std::string, T>
                                       || std::is_same_v<std::vector<std::string>, T>,
                                   void>> : std::true_type {};

enum class ArgType { BOOL, STR, LIST, INT, FLOAT };

struct ArgInfo {
    std::string  name;
    ArgType      type;
    std::string  help;
    std::string  sname;
    ArgValueType value;
    bool         has_value{false};

    static std::string type_str(ArgType t);

    std::string toString() const;
};

class ArgParser {
public:
    auto get_value(std::string_view arg) -> std::optional<ArgValueType>;
    auto add_arg(ArgInfo&& info) -> ArgParser&;
    auto get_default_args() -> std::optional<std::vector<std::string>>;
    auto get_file_name() -> std::string;
    auto parse(int argc, const char** argv) -> void;
    auto help() -> void;

private:
    std::string                    file_name;
    std::vector<std::string>       default_args;
    std::map<std::string, ArgInfo> arg_info_mp;
};

// Implementation

inline std::string ArgInfo::type_str(ArgType t) {
    switch (t) {
        case ArgType::INT:
            return "int";
        case ArgType::FLOAT:
            return "float";
        case ArgType::LIST:
            return "strlist";
        case ArgType::BOOL:
            return "bool";
        default:
            return "str";
    }
}

inline std::string ArgInfo::toString() const {
    return sk::utils::format("{} [{}] {}", (sname.empty() ? name : sname + ", " + name), ArgInfo::type_str(type), help);
}

inline ArgParser& ArgParser::add_arg(ArgInfo&& info) {
    if (info.name.empty() || info.name[0] != '-') {
        SK_ERROR("Arg name should start with '-'. The '{}' is skipped.", info.name);
    } else {
        arg_info_mp.emplace(info.name, info);
    }
    return *this;
}

inline std::optional<ArgValueType> ArgParser::get_value(std::string_view arg) {
    auto it = arg_info_mp.find(std::string(arg));
    if (it == arg_info_mp.end() || !it->second.has_value) {
        return std::nullopt;
    }
    return it->second.value;
}

inline auto ArgParser::get_file_name() -> std::string {
    return this->file_name;
}

inline auto ArgParser::get_default_args() -> std::optional<std::vector<std::string>> {
    if (this->default_args.empty()) {
        return std::nullopt;
    }
    return this->default_args;
}

inline void ArgParser::parse(int argc, const char** argv) {  // NOLINT
    this->arg_info_mp.emplace("-h", ArgInfo{.name = "-h", .type = ArgType::BOOL, .help = "Show This Message"});
    this->arg_info_mp.emplace("--help", ArgInfo{.name = "--help", .type = ArgType::BOOL, .help = "Show This Message"});

    this->file_name = argv[0];
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == "=") {
            continue;
        }
        for (const std::string& s : sk::utils::str::split(std::string(argv[i]), {'=', ' '})) {
            args.emplace_back(s);
        }
    }
    int  idx     = 0;
    auto arg_num = args.size();

    while (idx < arg_num && (arg_info_mp.find(args[idx])) == arg_info_mp.end()) {
        default_args.emplace_back(args[idx]);
        ++idx;
    }

    while (idx < arg_num) {
        auto it = arg_info_mp.find(args[idx]);
        if (it == arg_info_mp.end()) {
            SK_ERROR("Arg Parse Error: '{}' appears at invalid position, try to skip.", args[idx]);
            ++idx;
            continue;
        }
        switch (it->second.type) {
            case ArgType::INT:
                it->second.value     = std::stoi(args[++idx]);
                it->second.has_value = true;
                break;
            case ArgType::FLOAT:
                it->second.value     = std::stod(args[++idx]);
                it->second.has_value = true;
                break;
            case ArgType::BOOL:
                it->second.value     = true;
                it->second.has_value = true;
                break;
            case ArgType::STR:
                it->second.value     = std::string(args[++idx]);
                it->second.has_value = true;
                break;
            case ArgType::LIST:
                if (!it->second.has_value) {
                    it->second.value = std::vector<std::string>{};
                }
                idx++;
                while (idx < arg_num && (arg_info_mp.find(args[idx]) == arg_info_mp.end())) {
                    std::get<std::vector<std::string>>(it->second.value).emplace_back(args[idx++]);
                }
                idx--;
                if (!std::get<std::vector<std::string>>(it->second.value).empty()) {
                    it->second.has_value = true;
                }
                break;
            default:
                SK_ERROR("UnKnown ArgType. Check if you have modified all appearance of ArgType.");
        }
        ++idx;
    }
}

inline auto ArgParser::help() -> void {
    std::string helpinfo{
        sk::utils::format("[Usage]: ./{} [<options>] [<args>]\n", sk::utils::str::basenameWithoutExt(file_name))};
    for (const auto& entry : arg_info_mp) {
        helpinfo += "\t";
        helpinfo += entry.second.toString();
        helpinfo += "\n";
    }
    {
        GUARD_LOG;
        std::cout << helpinfo;
    }
}

}  // namespace sk::utils::arg

#endif  // SHUAIKAI_UTILS_ARGS_PARSER_H