#ifndef SK_UTILS_LOGGER_H
#define SK_UTILS_LOGGER_H

#include "printer.h"
#include "string_utils.h"

#define LOG_SEP ":"

#if SK_LOG_FOR_DEBUG
#define COUT_POSITION \
    "[" << sk::utils::str::basenameWithoutExt(__FILE__) << LOG_SEP << __FUNCTION__ << LOG_SEP << __LINE__ << "]"
#else
#define COUT_POSITION \
    "[" << sk::utils::time::current("%H:%M:%S") << "][" << sk::utils::str::basenameWithoutExt(__FILE__) << "]"
#endif

#define SK_LOG(...)                                                                                    \
    do {                                                                                               \
        auto msg__ = sk::utils::colorful_format(__VA_ARGS__);                                          \
        GUARD_LOG;                                                                                     \
        std::cout << ANSI_BLUE_BG << "[DEBUG]" << COUT_POSITION << " " << ANSI_CLEAR << msg__ << "\n"; \
    } while (0);

#define SK_WARN(...)                                                                                     \
    do {                                                                                                 \
        auto msg__ = sk::utils::colorful_format(__VA_ARGS__);                                            \
        GUARD_LOG;                                                                                       \
        std::cerr << ANSI_YELLOW_BG << "[ WARN]" << COUT_POSITION << " " << ANSI_CLEAR << msg__ << "\n"; \
    } while (0);

#define SK_ERROR(...)                                                                                 \
    do {                                                                                              \
        auto msg__ = sk::utils::colorful_format(__VA_ARGS__);                                         \
        GUARD_LOG;                                                                                    \
        std::cerr << ANSI_RED_BG << "[ERROR]" << COUT_POSITION << " " << ANSI_CLEAR << msg__ << "\n"; \
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

#endif  // SK_UTILS_LOGGER_H
