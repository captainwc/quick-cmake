#ifndef SK_UTILS_MACRO_H
#define SK_UTILS_MACRO_H

#include "config.h"            // for gFailedTest
#include "printer.h"           // for LOG_GUARD and toString utils
#include "string_utils.h"  // for replace to replace ELEM_SEP

/// MARK: TESTER

#define REPLACED_SEP(s) sk::utils::str::replace((s), ",", ELEM_SEP)

/// MARK: ERRNO

#define RETURN_TESTS_PASSED 0

#define RETURN_TESTS_FAILED 101
#define EXIT_ASSERT_FAIL    102
#define EXIT_PARAM_ILLEGAL  103
#define EXIT_CTRL_C         104

#define THREAD_SAFE_EXIT(x) exit(x)  // why asked thread-safe?

/// MARK: COUT

#define COUT(x)                                                                      \
    do {                                                                             \
        std::string ret = sk::utils::toString(x);                                    \
        GUARD_LOG;                                                                   \
        std::cout << ANSI_BLUE_BG << "[" #x "]: " << ANSI_CLEAR << ret << std::endl; \
    } while (0);

#define COUT_POSITION "[" << __FILE__ << ELEM_SEP << __FUNCTION__ << ELEM_SEP << __LINE__ << "]"

#define TODO(msg)                                                                                           \
    do {                                                                                                    \
        GUARD_LOG;                                                                                          \
        std::cerr << ANSI_YELLOW_BG << (msg) << ANSI_PURPLE_BG << COUT_POSITION << ANSI_CLEAR << std::endl; \
    } while (0);

#define FILL_ME() TODO("Fill Code Here!!! -> ")

#define LINE_BREAKER(msg)                                                                                  \
    do {                                                                                                   \
        GUARD_LOG;                                                                                         \
        std::cout << ANSI_YELLOW_BG << "========== " << (msg) << " ==========" << ANSI_CLEAR << std::endl; \
    } while (0);

#define NEW_LINE()              \
    do {                        \
        GUARD_LOG;              \
        std::cout << std::endl; \
    } while (0);

/// MARK: DUMP

#define TO_PAIR(x)                                           std::make_pair(#x, x)
#define DUMP1(x)                                             sk::utils::dumpWithName(TO_PAIR(x))
#define DUMP2(x, ...)                                        sk::utils::dumpWithName(TO_PAIR(x)), DUMP1(__VA_ARGS__)
#define DUMP3(x, ...)                                        sk::utils::dumpWithName(TO_PAIR(x)), DUMP2(__VA_ARGS__)
#define DUMP4(x, ...)                                        sk::utils::dumpWithName(TO_PAIR(x)), DUMP3(__VA_ARGS__)
#define DUMP5(x, ...)                                        sk::utils::dumpWithName(TO_PAIR(x)), DUMP4(__VA_ARGS__)
#define DUMP6(x, ...)                                        sk::utils::dumpWithName(TO_PAIR(x)), DUMP5(__VA_ARGS__)
#define DUMP7(x, ...)                                        sk::utils::dumpWithName(TO_PAIR(x)), DUMP6(__VA_ARGS__)
#define DUMP8(x, ...)                                        sk::utils::dumpWithName(TO_PAIR(x)), DUMP7(__VA_ARGS__)
#define GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, NAME, ...) NAME
#define DUMP(...)                                                                                    \
    do {                                                                                             \
        GET_MACRO(__VA_ARGS__, DUMP8, DUMP7, DUMP6, DUMP5, DUMP4, DUMP3, DUMP2, DUMP1)(__VA_ARGS__); \
        std::cout << "\n";                                                                           \
    } while (0);

/// MARK: ASSERT

#include <cassert>

#define ASSERT_MSG(expr, msg)                                             \
    do {                                                                  \
        if (!(expr)) {                                                    \
            GUARD_LOG;                                                    \
            std::cerr << ANSI_RED_BG << (msg) << ANSI_CLEAR << std::endl; \
            THREAD_SAFE_EXIT(EXIT_ASSERT_FAIL);                           \
        }                                                                 \
    } while (0)

#define ASSERT(expr)                                                                                      \
    do {                                                                                                  \
        if (!(expr)) {                                                                                    \
            GUARD_LOG;                                                                                    \
            std::cerr << ANSI_RED_BG << "Assert " << ANSI_PURPLE_BG << #expr << ANSI_RED_BG << " Failed!" \
                      << ANSI_CLEAR << std::endl;                                                         \
            THREAD_SAFE_EXIT(EXIT_ASSERT_FAIL);                                                           \
        }                                                                                                 \
    } while (0)

#define ASSERT_TRUE(expr)                                                                                          \
    do {                                                                                                           \
        if (expr) {                                                                                                \
            GUARD_LOG;                                                                                             \
            std::cout << ANSI_GREEN_BG << "[PASSED] " << ANSI_CLEAR                                                \
                      << "[" + std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + "] "       \
                      << ANSI_BLUE_BG << #expr << ANSI_GREEN_BG << " => true" << ANSI_CLEAR << std::endl;          \
        } else {                                                                                                   \
            ++sk::utils::GlobalInfo::getInstance().gFailedTest;                                                    \
            GUARD_LOG;                                                                                             \
            std::cerr << ANSI_RED_BG << "[FAILED] " << ANSI_CLEAR                                                  \
                      << "[" + std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + "] "       \
                      << ANSI_BLUE_BG << #expr << ANSI_RED_BG << " => istrue? " << ANSI_YELLOW_BG << COUT_POSITION \
                      << ANSI_CLEAR << std::endl;                                                                  \
            std::cerr << ANSI_PURPLE_BG << '\t' << "Expected: " << ANSI_CLEAR << "True" << std::endl;              \
            std::cerr << ANSI_PURPLE_BG << '\t' << "  Actual: " << ANSI_CLEAR << "False" << std::endl;             \
        }                                                                                                          \
        ++sk::utils::GlobalInfo::getInstance().gTotalTest;                                                         \
    } while (0)

/// x: expected, y: acutal
#define ASSERT_EQUAL(x, y)                                                                                    \
    do {                                                                                                      \
        if ((x) == (y)) {                                                                                     \
            GUARD_LOG;                                                                                        \
            std::cout << ANSI_GREEN_BG << "[PASSED] " << ANSI_CLEAR                                           \
                      << "[" + std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + "] "  \
                      << ANSI_BLUE_BG << #x << ANSI_GREEN_BG << " == " << ANSI_BLUE_BG << #y << ANSI_CLEAR    \
                      << std::endl;                                                                           \
        } else {                                                                                              \
            ++sk::utils::GlobalInfo::getInstance().gFailedTest;                                               \
            GUARD_LOG;                                                                                        \
            std::cerr << ANSI_RED_BG << "[FAILED] " << ANSI_CLEAR                                             \
                      << "[" + std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + "] "  \
                      << ANSI_BLUE_BG << #x << ANSI_RED_BG " == " << ANSI_BLUE_BG << #y " " << ANSI_YELLOW_BG \
                      << COUT_POSITION << ANSI_CLEAR << std::endl;                                            \
            std::cerr << ANSI_PURPLE_BG << '\t' << "Expected: " << ANSI_CLEAR << "Equal" << std::endl;        \
            std::cerr << ANSI_PURPLE_BG << '\t' << "  Actual: " << ANSI_CLEAR << "NonEqual" << std::endl;     \
        }                                                                                                     \
        ++sk::utils::GlobalInfo::getInstance().gTotalTest;                                                    \
    } while (0)

/// x: expected, y: acutal
#define STR_EQUAL_(x, y)                                                                                         \
    do {                                                                                                         \
        auto expected = sk::utils::toString(x);                                                                  \
        auto actual   = sk::utils::toString(y);                                                                  \
        if (expected == actual) {                                                                                \
            GUARD_LOG;                                                                                           \
            std::cout << ANSI_GREEN_BG << "[PASSED] " << ANSI_CLEAR                                              \
                      << "[" + std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + "] "     \
                      << ANSI_BLUE_BG << #x << ANSI_GREEN_BG << " == " << ANSI_BLUE_BG << #y << ANSI_CLEAR       \
                      << std::endl;                                                                              \
        } else {                                                                                                 \
            ++sk::utils::GlobalInfo::getInstance().gFailedTest;                                                  \
            GUARD_LOG;                                                                                           \
            std::cerr << ANSI_RED_BG << "[FAILED] " << ANSI_CLEAR                                                \
                      << "[" + std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + "] "     \
                      << ANSI_BLUE_BG << #x << ANSI_RED_BG << " == " << ANSI_BLUE_BG << #y " " << ANSI_YELLOW_BG \
                      << COUT_POSITION << ANSI_CLEAR << std::endl;                                               \
            std::cerr << ANSI_PURPLE_BG << '\t' << "Expected: " << ANSI_CLEAR << expected << std::endl;          \
            std::cerr << ANSI_PURPLE_BG << '\t' << "  Actual: " << ANSI_CLEAR << actual << std::endl;            \
        }                                                                                                        \
        ++sk::utils::GlobalInfo::getInstance().gTotalTest;                                                       \
    } while (0)

#define ASSERT_STR_EQUAL(x, y) STR_EQUAL_(x, y)

inline int ASSERT_ALL_PASSED() {
    GUARD_LOG;
    std::cout << std::endl;
    if (sk::utils::GlobalInfo::getInstance().gFailedTest.load() == 0) {
        std::cout << ANSI_GREEN_BG << "==== "
                  << std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + "/"
                         + std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + " PASSED ALL! ===="
                  << ANSI_CLEAR << std::endl;
        return RETURN_TESTS_PASSED;
    }
    std::cout << ANSI_RED_BG << "==== "
              << std::to_string(sk::utils::GlobalInfo::getInstance().gFailedTest.load()) + "/"
                     + std::to_string(sk::utils::GlobalInfo::getInstance().gTotalTest.load()) + " test failed! ===="
              << ANSI_CLEAR << std::endl;
    return RETURN_TESTS_FAILED;
}

#endif  // SK_UTILS_MACRO_H
