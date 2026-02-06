#ifndef SK_UTILS_STRING_UTILS_H
#define SK_UTILS_STRING_UTILS_H

#include <algorithm>
#include <cctype>
#include <functional>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

namespace sk::utils::str {
inline bool startWith(std::string_view str, std::string_view prefix) {
#if __cplusplus >= 202002L
  return str.starts_with(prefix);
#else
  if (str.length() < prefix.length()) {
    return false;
  }
  for (int i = 0; i < prefix.length(); i++) {
    if (prefix[i] != str[i]) {
      return false;
    }
  }
  return true;
#endif
}

inline bool endWith(std::string_view str, std::string_view suffix) {
#if __cplusplus >= 202002L
  return str.ends_with(suffix);
#else
  if (str.length() < suffix.length()) {
    return false;
  }
  auto offset = str.length() - suffix.length();
  for (int i = 0; i < suffix.length(); i++) {
    if (str[i + offset] != suffix[i]) {
      return false;
    }
  }
  return true;
#endif
}

inline std::string strip(std::string_view str) {
  auto l = str.find_first_not_of(' ');
  auto r = str.find_last_not_of(' ');
  return std::string(str.substr(l, r - l + 1));
}

inline bool isspace(char c) {
  return c == ' ' || c == '\n' || c == '\t';
}

inline bool isspace(std::string_view s) {
  return std::all_of(s.begin(), s.end(), [](char c) { return isspace(c); });
}

inline bool contains(std::string_view str, std::string_view substr) {
  return str.find(substr) != std::string::npos;
}

inline int constexpr count(std::string_view str, std::string_view substr) {
  int cnt = 0;
  size_t pos = 0;
  auto sublength = substr.length();
  while ((pos = str.find(substr, pos)) != std::string::npos) {
    cnt++;
    pos += sublength;
  }
  return cnt;
}

template <typename IterMoter>
inline auto _split(std::string_view str, IterMoter itermoter) -> typename std::enable_if<
  std::is_same<decltype(itermoter(std::declval<std::string_view>())), std::pair<bool, decltype(str.length())>>::value,
  std::vector<std::string>>::type {
  auto len = str.length();
  decltype(len) idx = 0;
  decltype(len) starter = 0;
  std::vector<std::string> ret;
  while (idx < len) {
    auto [isDelim, idxInstance] = itermoter(str.substr(idx, len - idx + 1));
    if (isDelim) {
      if (idx > starter) {
        ret.emplace_back(str.substr(starter, idx - starter));
      }
      idx += idxInstance;
      starter = idx;
    } else {
      ++idx;
    }
  }
  if (starter < len) {
    ret.emplace_back(str.substr(starter, len - starter + 1));
  }
  return ret;
}

inline std::vector<std::string> split(std::string_view str, char delim) {
  return _split(str, [&delim](std::string_view substr) -> std::pair<bool, decltype(str.length())> {
    if (substr[0] == delim) {
      return {true, 1};
    }
    return {false, 0};
  });
}

inline std::vector<std::string> split(std::string_view str, std::string_view delim) {
  return _split(str, [&](std::string_view substr) -> std::pair<bool, decltype(str.length())> {
    if (startWith(substr, delim)) {
      return {true, delim.length()};
    }
    return {false, 0};
  });
}

inline std::vector<std::string> split(std::string_view str, std::vector<std::string> delims) {
  std::sort(delims.begin(), delims.end(), std::greater<>());
  return _split(str, [&](std::string_view substr) -> std::pair<bool, decltype(str.length())> {
    for (const auto& delim : delims) {
      if (startWith(substr, delim)) {
        return {true, delim.length()};
      }
    }
    return {false, 0};
  });
}

inline std::vector<std::string> split(std::string_view str, std::vector<char> delims) {
  std::sort(delims.begin(), delims.end(), std::greater<>());
  return _split(str, [&](std::string_view substr) -> std::pair<bool, decltype(str.length())> {
    for (char c : delims) {
      if (substr[0] == c) {
        return {true, 1};
      }
    }
    return {false, 0};
  });
}

inline std::string replace(const std::string& str, const std::string& pattern, const std::string& replacer) {
  std::regex re(pattern);
  return std::regex_replace(str, re, replacer);
}

inline std::string replace(std::string&& str, std::string&& pattern, std::string&& replacer) {
  std::regex re(pattern);
  return std::regex_replace(str, re, replacer);
}

inline std::string trim(const std::string str, const std::string trimer = " ") {
  return replace(str, trimer, "");
}

inline std::string dirname(std::string_view filename) {
  auto pos = filename.find_last_of('/');
  if (pos == std::string::npos) {
    pos = filename.find_last_of('\\');
  }
  return std::string(filename.substr(0, pos));
}

inline std::string basename(std::string_view filename) {
  return *(split(filename, std::vector<std::string>{"/", "\\"}).end() - 1);
}

inline std::string basenameWithoutExt(std::string_view filename) {
  auto base = basename(filename);
  return base.substr(0, base.find_last_of('.'));
}

inline std::string expandUser(std::string_view path) {
  if (!startWith(path, "~")) {
    return std::string(path);
  }
  char* home_dir = nullptr;
#if defined(_WIN32)
  home_dir = getenv("USERPROFILE");
#else
  home_dir = getenv("HOME");
#endif

  return replace(std::string(path), "~", std::string(home_dir));
}

}  // namespace sk::utils::str

#endif  // SK_UTILS_STRING_UTILS_H
