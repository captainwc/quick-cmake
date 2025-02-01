#ifndef SK_UTILS_MACRO_H
#define SK_UTILS_MACRO_H

#include <chrono>
#include <functional>
#include <utility>

#include "config.h"        // for gFailedTest
#include "printer.h"       // for LOG_GUARD and toString utils
#include "string_utils.h"  // for replace to replace ELEM_SEP, and basenameWithoutExt
#include "time_utils.h"    // for current()

/// MARK: COLOR

#define WITH_RED(x)    "\033[0m\033[0;31m" + sk::utils::toString(x) + "\033[0m"
#define WITH_GREEN(x)  "\033[0m\033[0;32m" + sk::utils::toString(x) + "\033[0m"
#define WITH_YELLOW(x) "\033[0m\033[0;33m" + sk::utils::toString(x) + "\033[0m"
#define WITH_BLUE(x)   "\033[0m\033[0;34m" + sk::utils::toString(x) + "\033[0m"
#define WITH_PURPLE(x) "\033[0m\033[0;35m" + sk::utils::toString(x) + "\033[0m"
#define WITH_GRAY(x)   "\033[0m\033[38;5;246m" + sk::utils::toString(x) + "\033[0m"
#define WITH_BOLD(x)   "\033[0m\033[1m" + sk::utils::toString(x) + "\033[0m"
#define WITH_ITALIC(x) "\033[0m\033[3m" + sk::utils::toString(x) + "\033[0m"

/// MARK: TESTER

#define REPLACED_SEP(s) sk::utils::str::replace((s), ",", ELEM_SEP)

/// MARK: ERRNO

#define RETURN_TESTS_PASSED 0

#define RETURN_TESTS_FAILED 101
#define EXIT_ASSERT_FAIL    102
#define EXIT_PARAM_ILLEGAL  103
#define EXIT_CTRL_C         104

#define THREAD_SAFE_EXIT(x) exit(x)  // why asked thread-safe?

/// MARK: Logger
#define LOG_SEP ":"

#if SK_LOG_FOR_DEBUG
#define COUT_POSITION \
    "[" << sk::utils::str::basenameWithoutExt(__FILE__) << LOG_SEP << __FUNCTION__ << LOG_SEP << __LINE__ << "]"
#else
#define COUT_POSITION \
    "[" << sk::utils::time::current("%H:%M:%S") << "][" << sk::utils::str::basenameWithoutExt(__FILE__) << "]"
#endif

#define SK_LOG(...)                                                                                  \
    do {                                                                                             \
        auto msg = sk::utils::colorful_format(__VA_ARGS__);                                          \
        GUARD_LOG;                                                                                   \
        std::cout << ANSI_BLUE_BG << "[DEBUG]" << COUT_POSITION << " " << ANSI_CLEAR << msg << "\n"; \
    } while (0);

#define SK_WARN(...)                                                                                   \
    do {                                                                                               \
        auto msg = sk::utils::colorful_format(__VA_ARGS__);                                            \
        GUARD_LOG;                                                                                     \
        std::cerr << ANSI_YELLOW_BG << "[ WARN]" << COUT_POSITION << " " << ANSI_CLEAR << msg << "\n"; \
    } while (0);

#define SK_ERROR(...)                                                                               \
    do {                                                                                            \
        auto msg = sk::utils::colorful_format(__VA_ARGS__);                                         \
        GUARD_LOG;                                                                                  \
        std::cerr << ANSI_RED_BG << "[ERROR]" << COUT_POSITION << " " << ANSI_CLEAR << msg << "\n"; \
    } while (0);

#define TODO(msg)                                                                                                  \
    do {                                                                                                           \
        GUARD_LOG;                                                                                                 \
        std::cerr << ANSI_YELLOW_BG << "[TODO]" << ANSI_BLUE_BG << COUT_POSITION << ":" << ANSI_PURPLE_BG << (msg) \
                  << ANSI_CLEAR << "\n";                                                                           \
    } while (0);

#define FILL_ME() TODO("<== Fill Code Here!!! ")

#define LINE_BREAKER(msg)                                                                             \
    do {                                                                                              \
        GUARD_LOG;                                                                                    \
        std::cout << ANSI_YELLOW_BG << "========== " << (msg) << " ==========" << ANSI_CLEAR << "\n"; \
    } while (0);

#define NEW_LINE()         \
    do {                   \
        GUARD_LOG;         \
        std::cout << "\n"; \
    } while (0);

/// MARK: DUMP

#define TO_PAIR(x) std::make_pair(#x, x)

#define DUMP1(x)      sk::utils::dumpWithName(TO_PAIR(x))
#define DUMP2(x, ...) sk::utils::dumpWithName(TO_PAIR(x)), DUMP1(__VA_ARGS__)
#define DUMP3(x, ...) sk::utils::dumpWithName(TO_PAIR(x)), DUMP2(__VA_ARGS__)
#define DUMP4(x, ...) sk::utils::dumpWithName(TO_PAIR(x)), DUMP3(__VA_ARGS__)
#define DUMP5(x, ...) sk::utils::dumpWithName(TO_PAIR(x)), DUMP4(__VA_ARGS__)
#define DUMP6(x, ...) sk::utils::dumpWithName(TO_PAIR(x)), DUMP5(__VA_ARGS__)
#define DUMP7(x, ...) sk::utils::dumpWithName(TO_PAIR(x)), DUMP6(__VA_ARGS__)
#define DUMP8(x, ...) sk::utils::dumpWithName(TO_PAIR(x)), DUMP7(__VA_ARGS__)

#define GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, NAME, ...) NAME
#define DUMP(...)                                                                                    \
    do {                                                                                             \
        GET_MACRO(__VA_ARGS__, DUMP8, DUMP7, DUMP6, DUMP5, DUMP4, DUMP3, DUMP2, DUMP1)(__VA_ARGS__); \
        std::cout << "\n";                                                                           \
    } while (0);

/// MARK: ASSERT

#include <cassert>

#define ASSERT_MSG(expr, msg)                                        \
    do {                                                             \
        if (!(expr)) {                                               \
            GUARD_LOG;                                               \
            std::cerr << ANSI_RED_BG << (msg) << ANSI_CLEAR << "\n"; \
            THREAD_SAFE_EXIT(EXIT_ASSERT_FAIL);                      \
        }                                                            \
    } while (0)

#define ASSERT(expr)                                                                                      \
    do {                                                                                                  \
        if (!(expr)) {                                                                                    \
            GUARD_LOG;                                                                                    \
            std::cerr << ANSI_RED_BG << "Assert " << ANSI_PURPLE_BG << #expr << ANSI_RED_BG << " Failed!" \
                      << ANSI_CLEAR << "\n";                                                              \
            THREAD_SAFE_EXIT(EXIT_ASSERT_FAIL);                                                           \
        }                                                                                                 \
    } while (0)

#define ASSERT_TRUE(expr)                                                                                          \
    do {                                                                                                           \
        if (expr) {                                                                                                \
            GUARD_LOG;                                                                                             \
            std::cout << ANSI_GREEN_BG << "[PASSED] " << ANSI_CLEAR                                                \
                      << "[" + std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + "] "       \
                      << ANSI_BLUE_BG << #expr << ANSI_GREEN_BG << " => true" << ANSI_CLEAR << "\n";               \
        } else {                                                                                                   \
            ++sk::utils::GlobalInfo::getInstance().gFailedTest;                                                    \
            GUARD_LOG;                                                                                             \
            std::cerr << ANSI_RED_BG << "[FAILED] " << ANSI_CLEAR                                                  \
                      << "[" + std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + "] "       \
                      << ANSI_BLUE_BG << #expr << ANSI_RED_BG << " => istrue? " << ANSI_YELLOW_BG << COUT_POSITION \
                      << ANSI_CLEAR << "\n";                                                                       \
            std::cerr << ANSI_PURPLE_BG << '\t' << "Expected: " << ANSI_CLEAR << "True" << "\n";                   \
            std::cerr << ANSI_PURPLE_BG << '\t' << "  Actual: " << ANSI_CLEAR << "False" << "\n";                  \
        }                                                                                                          \
        ++sk::utils::GlobalInfo::getInstance().gTotalTest;                                                         \
    } while (0)

/// x: expected, y: acutal
#define ASSERT_EQUAL(x, y)                                                                                          \
    do {                                                                                                            \
        if ((x) == (y)) {                                                                                           \
            GUARD_LOG;                                                                                              \
            std::cout << ANSI_GREEN_BG << "[PASSED] " << ANSI_CLEAR                                                 \
                      << "[" + std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + "] "        \
                      << ANSI_BLUE_BG << #x << ANSI_GREEN_BG << " == " << ANSI_BLUE_BG << #y << ANSI_CLEAR << "\n"; \
        } else {                                                                                                    \
            ++sk::utils::GlobalInfo::getInstance().gFailedTest;                                                     \
            GUARD_LOG;                                                                                              \
            std::cerr << ANSI_RED_BG << "[FAILED] " << ANSI_CLEAR                                                   \
                      << "[" + std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + "] "        \
                      << ANSI_BLUE_BG << #x << ANSI_RED_BG " == " << ANSI_BLUE_BG << #y " " << ANSI_YELLOW_BG       \
                      << COUT_POSITION << ANSI_CLEAR << "\n";                                                       \
            std::cerr << ANSI_PURPLE_BG << '\t' << "Expected: " << ANSI_CLEAR << "Equal" << "\n";                   \
            std::cerr << ANSI_PURPLE_BG << '\t' << "  Actual: " << ANSI_CLEAR << "NonEqual" << "\n";                \
        }                                                                                                           \
        ++sk::utils::GlobalInfo::getInstance().gTotalTest;                                                          \
    } while (0)

/// x: expected, y: acutal
#define STR_EQUAL_(x, y)                                                                                            \
    do {                                                                                                            \
        auto expected = sk::utils::toString(x);                                                                     \
        auto actual   = sk::utils::toString(y);                                                                     \
        if (expected == actual) {                                                                                   \
            GUARD_LOG;                                                                                              \
            std::cout << ANSI_GREEN_BG << "[PASSED] " << ANSI_CLEAR                                                 \
                      << "[" + std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + "] "        \
                      << ANSI_BLUE_BG << #x << ANSI_GREEN_BG << " == " << ANSI_BLUE_BG << #y << ANSI_CLEAR << "\n"; \
        } else {                                                                                                    \
            ++sk::utils::GlobalInfo::getInstance().gFailedTest;                                                     \
            GUARD_LOG;                                                                                              \
            std::cerr << ANSI_RED_BG << "[FAILED] " << ANSI_CLEAR                                                   \
                      << "[" + std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + "] "        \
                      << ANSI_BLUE_BG << #x << ANSI_RED_BG << " == " << ANSI_BLUE_BG << #y " " << ANSI_YELLOW_BG    \
                      << COUT_POSITION << ANSI_CLEAR << "\n";                                                       \
            std::cerr << ANSI_PURPLE_BG << '\t' << "Expected: " << ANSI_CLEAR << expected << "\n";                  \
            std::cerr << ANSI_PURPLE_BG << '\t' << "  Actual: " << ANSI_CLEAR << actual << "\n";                    \
        }                                                                                                           \
        ++sk::utils::GlobalInfo::getInstance().gTotalTest;                                                          \
    } while (0)

#define ASSERT_STR_EQUAL(x, y) STR_EQUAL_(x, y)

inline int ASSERT_ALL_PASSED() {
    GUARD_LOG;
    std::cout << "\n";
    if (sk::utils::GlobalInfo::getInstance().gFailedTest.load() == 0) {
        std::cout << ANSI_GREEN_BG << "==== "
                  << std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + "/"
                         + std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + " PASSED ALL! ===="
                  << ANSI_CLEAR << "\n";
        return RETURN_TESTS_PASSED;
    }
    std::cout << ANSI_RED_BG << "==== "
              << std::to_string(sk::utils::GlobalInfo::getInstance().gFailedTest.load()) + "/"
                     + std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + " test failed! ===="
              << ANSI_CLEAR << "\n";
    return RETURN_TESTS_FAILED;
}

#endif  // SK_UTILS_MACRO_H
