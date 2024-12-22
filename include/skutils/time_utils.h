#ifndef SHUAIKAI_UTILS_TIME_UTILS_H
#define SHUAIKAI_UTILS_TIME_UTILS_H

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

namespace sk::utils::time {

inline std::string current(const char* format = "%Y-%m-%d %H:%M:%S") {
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    std::time_t       time       = std::chrono::system_clock::to_time_t(now);
    std::tm*          local_time = std::localtime(&time);
    std::stringstream ss;
    ss << std::put_time(local_time, format);
    return ss.str();
}

}  // namespace sk::utils::time

#endif  // SHUAIKAI_UTILS_TIME_UTILS_H