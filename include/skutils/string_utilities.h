#ifndef SK_UTILS_STRING_UTILS_H
#define SK_UTILS_STRING_UTILS_H

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

inline bool contains(std::string_view str, std::string_view substr) {
    return str.find(substr) != std::string::npos;
}

int constexpr count(std::string_view str, std::string_view substr) {
    int    cnt       = 0;
    size_t pos       = 0;
    auto   sublength = substr.length();
    while ((pos = str.find(substr, pos)) != std::string::npos) {
        cnt++;
        pos += sublength;
    }
    return cnt;
}

template <typename IterMoter>
auto _split(std::string_view str, IterMoter itermoter) -> typename std::enable_if<
    std::is_same<decltype(itermoter(std::declval<std::string_view>())), std::pair<bool, decltype(str.length())>>::value,
    std::vector<std::string>>::type {
    auto                     len     = str.length();
    decltype(len)            idx     = 0;
    decltype(len)            starter = 0;
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

}  // namespace sk::utils::str

#endif  // SK_UTILS_STRING_UTILS_H
