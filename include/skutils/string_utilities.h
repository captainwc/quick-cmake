#ifndef SK_UTILS_STRING_UTILS_H
#define SK_UTILS_STRING_UTILS_H

#include <string>
#include <string_view>

namespace sk::utils::str {
inline bool startWith(std::string_view str, std::string_view prefix) {
    return str.starts_with(prefix);
}

inline bool endWith(std::string_view str, std::string_view suffix) {
    return str.ends_with(suffix);
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
}  // namespace sk::utils::str

#endif  // SK_UTILS_STRING_UTILS_H