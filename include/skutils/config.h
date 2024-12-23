#ifndef SHUAIKAI_CONFIG_H
#define SHUAIKAI_CONFIG_H

#include <atomic>
#include <iostream>

#include "spinlock.h"  // for spinlock

namespace sk::utils {

#define ELEM_SEP ","
#define DUMP_SEP "\n"

#define UNKNOWN_TYPE_STRING "<?>"

class GlobalInfo {
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

    GlobalInfo(const GlobalInfo&)            = delete;
    GlobalInfo& operator=(const GlobalInfo&) = delete;
};

}  // namespace sk::utils

#endif  // SHUAIKAI_CONFIG_H