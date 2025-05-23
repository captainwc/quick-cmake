#ifndef SK_UTILS_MACRO_H
#define SK_UTILS_MACRO_H

#include <chrono>
#include <functional>
#include <utility>

#include "string_utils.h"  // for replace to replace ELEM_SEP, and basenameWithoutExt

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

#endif  // SK_UTILS_MACRO_H
