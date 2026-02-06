#ifndef SHUAIKAI_UTILS_ARGS_PARSER_H
#define SHUAIKAI_UTILS_ARGS_PARSER_H

#include <algorithm>
#include <exception>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include "logger.h"
#include "printer.h"
#include "string_utils.h"

namespace sk::utils::arg {

#define COLLECT_UNEXPECTED_ARGS 1  // True means that collect all follow args when met an invalidate args

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
  std::string name;
  ArgType type;
  std::string help;
  std::string sname;
  ArgValueType value;
  bool has_value{false};

  static std::string type_str(ArgType t);

  std::string toString() const;
};

class ArgParser {
  public:
  auto get_value(std::string_view arg) -> std::optional<ArgValueType>;
  auto get_value_with_default(std::string_view arg) -> std::optional<std::vector<std::string>>;
  auto add_arg(ArgInfo&& info) -> ArgParser&;
  auto get_front_args() -> std::optional<std::vector<std::string>>;
  auto get_back_args() -> std::optional<std::vector<std::string>>;
  auto get_file_name() -> std::string;
  auto parse(int argc, char* argv[]) -> void;
  auto need_help() -> bool;
  auto show_help() -> void;

  private:
  std::string file_name;
  std::vector<std::string> front_default_args;
  std::vector<std::string> back_default_args;
  std::map<std::string, ArgInfo> arg_info_mp;
};

// Implementation

inline std::string ArgInfo::type_str(ArgType t) {
  switch (t) {
    case ArgType::INT: return "int";
    case ArgType::FLOAT: return "float";
    case ArgType::LIST: return "strlist";
    case ArgType::BOOL: return "bool";
    default: return "str";
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

inline auto ArgParser::get_value(std::string_view arg) -> std::optional<ArgValueType> {
  auto it = arg_info_mp.find(std::string(arg));
  if (it == arg_info_mp.end() || !it->second.has_value) {
    return std::nullopt;
  }
  return std::optional<ArgValueType>{it->second.value};
}

inline auto ArgParser::get_value_with_default(std::string_view arg) -> std::optional<std::vector<std::string>> {
  auto it = arg_info_mp.find(std::string(arg));
  if (it == arg_info_mp.end()) {
    return std::nullopt;
  }
  if (it->second.type != ArgType::LIST) {
    return std::nullopt;
  }
  auto pv = std::get<std::vector<std::string>>(get_value(arg).value_or(std::vector<std::string>()));
  auto fv = get_front_args().value_or(std::vector<std::string>());
  auto bv = get_back_args().value_or(std::vector<std::string>());
  std::vector<std::string> ret;
  ret.reserve((pv.size() + fv.size() + bv.size()));
  for (auto& p : pv) {
    ret.emplace_back(std::move(p));
  }
  for (auto& p : fv) {
    ret.emplace_back(std::move(p));
  }
  for (auto& p : bv) {
    ret.emplace_back(std::move(p));
  }
  if (ret.empty()) {
    return std::nullopt;
  }
  return std::optional<std::vector<std::string>>{ret};
};

inline auto ArgParser::get_file_name() -> std::string {
  return this->file_name;
}

inline auto ArgParser::get_front_args() -> std::optional<std::vector<std::string>> {
  if (this->front_default_args.empty()) {
    return std::nullopt;
  }
  return std::optional<std::vector<std::string>>{this->front_default_args};
}

inline auto ArgParser::get_back_args() -> std::optional<std::vector<std::string>> {
  if (this->back_default_args.empty()) {
    return std::nullopt;
  }
  return std::optional<std::vector<std::string>>{this->back_default_args};
}

inline void ArgParser::parse(int argc, char* argv[]) {  // NOLINT
  this->arg_info_mp.emplace("-h", ArgInfo{.name = "-h", .type = ArgType::BOOL, .help = "Show This Message"});
  this->arg_info_mp.emplace("--help", ArgInfo{.name = "--help", .type = ArgType::BOOL, .help = "Show This Message"});

  this->file_name = argv[0];
  std::vector<std::string> args;
  for (int i = 1; i < argc; ++i) {
    if (argv[i] == "=") {
      continue;
    }
    for (const std::string& s : sk::utils::str::split(std::string(argv[i]), '=')) {
      args.emplace_back(s);
    }
  }

  args.erase(std::remove_if(args.begin(), args.end(), [](const auto& s) { return s == ""; }), args.end());
  std::transform(args.begin(), args.end(), args.begin(), sk::utils::str::strip);

  int idx = 0;
  auto arg_num = args.size();

  while (idx < arg_num && (arg_info_mp.find(args[idx])) == arg_info_mp.end()) {
    front_default_args.emplace_back(args[idx]);
    ++idx;
  }

  while (idx < arg_num) {
    auto it = arg_info_mp.find(args[idx]);
    if (it == arg_info_mp.end()) {
#if COLLECT_UNEXPECTED_ARGS
      while (idx < arg_num) {
        back_default_args.emplace_back(args[idx++]);
      }
      break;
#else
      SK_ERROR("Arg Parse Error: '{}' appears at invalid position, try to skip.", args[idx++]);
      continue;
#endif
    }

#define PARSE_ARGUMENT(parse_func)                                                                           \
  if ((idx + 1) >= arg_num) {                                                                                \
    SK_ERROR("Missing [{}] Value of \"{}\". Try to ignore this option.", ArgInfo::type_str(it->second.type), \
             it->first);                                                                                     \
  } else {                                                                                                   \
    try {                                                                                                    \
      it->second.value = parse_func(args[++idx]);                                                            \
      it->second.has_value = true;                                                                           \
    } catch (const std::exception& e) {                                                                      \
      SK_ERROR("Invalidate [{}] Value of \"{}\", provided \"{}\". Try to ignore this option.",               \
               ArgInfo::type_str(it->second.type), it->first, args[idx--]);                                  \
    }                                                                                                        \
  }

    switch (it->second.type) {
      case ArgType::INT: PARSE_ARGUMENT(std::stoi); break;
      case ArgType::FLOAT: PARSE_ARGUMENT(std::stod); break;
      case ArgType::STR: PARSE_ARGUMENT([](const std::string& s) { return s; }); break;
#undef PARSE_ARGUMENT
      case ArgType::BOOL:
        it->second.value = true;
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
      default: SK_ERROR("UnKnown ArgType. Check if you have modified all appearance of ArgType.");
    }
    ++idx;
  }
}

inline auto ArgParser::need_help() -> bool {
  return get_value("-h").has_value() || get_value("--help").has_value();
}

inline auto ArgParser::show_help() -> void {
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
