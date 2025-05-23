#ifndef SK_UTILS_TIME_MEASURE_H
#define SK_UTILS_TIME_MEASURE_H

#include <cassert>
#include <chrono>
#include <cstring>
#include <memory>
#include <ratio>
#include <vector>

#include "macro.h"
#include "printer.h"
#include "string_utils.h"

#define RETURN_TESTS_PASSED 0

#define RETURN_TESTS_FAILED 101
#define EXIT_ASSERT_FAIL    102
#define EXIT_PARAM_ILLEGAL  103
#define EXIT_CTRL_C         104

#define THREAD_SAFE_EXIT(x) exit(x)  // why asked thread-safe?

namespace sk::utils::test {

class TimerBase {
public:
    using ClockType = std::chrono::high_resolution_clock;
    using DurType   = std::chrono::duration<long, std::ratio<1, 1000000000>>;

    void start_measure() { start = ClockType::now(); }

    virtual void measured_body(){};

    void end_measure() {
        end = ClockType::now();
        format_result();
    }

    virtual void dump_result() {
        {
            GUARD_LOG;
            std::cout << ANSI_PURPLE_BG << "[TIME]" << ANSI_GREEN_BG << "(" << name << ")" << ANSI_RED_BG << " "
                      << formated_costs << suffix << ANSI_CLEAR << "\n";
        }
    }

    explicit TimerBase(const char* name) : name(name) {}

    virtual ~TimerBase() = default;

protected:
    const char* name;

    std::chrono::time_point<ClockType, DurType> start;
    std::chrono::time_point<ClockType, DurType> end;
    double                                      formated_costs;
    std::string                                 suffix;

    void format_result() {
        auto cost = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        if (cost >= 1000000000) {
            formated_costs = static_cast<double>(cost) / 1000000000.0;
            suffix         = "s";
        } else if (cost >= 1000000) {
            formated_costs = static_cast<double>(cost) / 1000000.0;
            suffix         = "ms";
        } else if (cost >= 1000) {
            formated_costs = static_cast<double>(cost) / 1000.0;
            suffix         = "us";
        } else {
            formated_costs = static_cast<double>(cost);
            suffix         = "ns";
        }
    }
};

class ScopedTimer : public TimerBase {
public:
    ScopedTimer(const char* name, const char* file, const char* func, int line)
        : TimerBase(name), file(file), func(func), line(line) {
        start_measure();
    }

    void dump_result() override {
        auto base_name = sk::utils::str::basename(file);
        {
            GUARD_LOG;
            std::cout << ANSI_PURPLE_BG << "[TIME]" << ANSI_GREEN_BG << "(" << name << ")" << ANSI_RED_BG << " "
                      << formated_costs << suffix << ANSI_GRAY_BG "    [" << base_name << ":" << func << ":" << line
                      << "]" << ANSI_CLEAR << "\n";
        }
    }

    ~ScopedTimer() override {
        end_measure();
        dump_result();
    }

private:
    const char* file;
    const char* func;
    int         line;
};

}  // namespace sk::utils::test

#define SCOPED_TIMER(_timer_case_name) \
    auto _tmp_scope_meature = sk::utils::test::ScopedTimer(_timer_case_name, __FILE__, __func__, __LINE__);

#define MEASURE_TIME(_timer_case_name)                                                           \
    class TimeMeasurer__##_timer_case_name : public sk::utils::test::TimerBase {                 \
    public:                                                                                      \
        TimeMeasurer__##_timer_case_name(const char* name) : sk::utils::test::TimerBase(name) {} \
        void measured_body() override;                                                           \
        ~TimeMeasurer__##_timer_case_name() {                                                    \
            start_measure();                                                                     \
            measured_body();                                                                     \
            end_measure();                                                                       \
            dump_result();                                                                       \
        }                                                                                        \
    } _tmp_time_measure_##_timer_case_name(#_timer_case_name);                                   \
    void TimeMeasurer__##_timer_case_name::measured_body()

#define RUN_DEMO(func, ...)                                                                                     \
    do {                                                                                                        \
        int demoid = sk::utils::GlobalInfo::getInstance().gDemoId.fetch_add(1);                                 \
        {                                                                                                       \
            GUARD_LOG;                                                                                          \
            std::cout << ANSI_YELLOW_BG << sk::utils::format("DEMO[{}] BEGIN ", demoid) << ANSI_BLUE_BG         \
                      << "====== FUNC [" << ANSI_PURPLE_BG << #func << ANSI_BLUE_BG << "] ======" << ANSI_CLEAR \
                      << "\n";                                                                                  \
        }                                                                                                       \
        auto start = std::chrono::high_resolution_clock::now();                                                 \
        func(__VA_ARGS__);                                                                                      \
        auto end = std::chrono::high_resolution_clock::now();                                                   \
        {                                                                                                       \
            GUARD_LOG;                                                                                          \
            std::cout << ANSI_YELLOW_BG << sk::utils::format("DEMO[{}]   END ", demoid) << ANSI_BLUE_BG         \
                      << "====== TIME [" << ANSI_PURPLE_BG                                                      \
                      << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " us"    \
                      << ANSI_BLUE_BG << "] ======" << ANSI_CLEAR << "\n";                                      \
        }                                                                                                       \
    } while (0)

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
            std::cerr << ANSI_PURPLE_BG << '\t' << "Expected: " << ANSI_CLEAR << "True"                            \
                      << "\n";                                                                                     \
            std::cerr << ANSI_PURPLE_BG << '\t' << "  Actual: " << ANSI_CLEAR << "False"                           \
                      << "\n";                                                                                     \
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
            std::cerr << ANSI_PURPLE_BG << '\t' << "Expected: " << ANSI_CLEAR << "Equal"                            \
                      << "\n";                                                                                      \
            std::cerr << ANSI_PURPLE_BG << '\t' << "  Actual: " << ANSI_CLEAR << "NonEqual"                         \
                      << "\n";                                                                                      \
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

#endif  // !SK_UTILS_TIME_MEASURE_H
