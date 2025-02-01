#ifndef SHUAIKAI_CONFIG_H
#define SHUAIKAI_CONFIG_H

#include <atomic>
#include <cstring>
#include <iostream>

#include "noncopyable.h"
#include "spinlock.h"  // for spinlock

namespace sk::utils {

#define ELEM_SEP ","
#define DUMP_SEP "\n"

#define SK_LOG_FOR_DEBUG 1  // set to 1 to print line infomation in log

#define UNKNOWN_TYPE_STRING "<?>"

enum class OSTYPE { Windows, Linux, Unknown };

inline constexpr OSTYPE OS_TYPE() {
#if defined(_WIN32) || defined(_WIN64)
    return OSTYPE::Windows;
#elif defined(__linux__)
    return OSTYPE::Linux;
#else
    return OSTYPE::Unknown;
#endif
}

inline constexpr bool IS_LINUX_OS() {
    return OS_TYPE() == OSTYPE::Linux;
}

class GlobalInfo : public NonCopyable {
private:
    GlobalInfo() : gFailedTest{0}, gTotalTest{0} {};

public:
    SpinLock         globalLogSpinLock;
    std::atomic<int> gFailedTest;
    std::atomic<int> gTotalTest;

    static GlobalInfo& getInstance() {
        static GlobalInfo instance;
        return instance;
    }
};

}  // namespace sk::utils

#endif  // SHUAIKAI_CONFIG_H
