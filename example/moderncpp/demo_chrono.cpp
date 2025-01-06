#include <algorithm>
#include <chrono>
#include <ratio>

#include "skutils/macro.h"
#include "skutils/printer.h"

using namespace std::literals;

/**
 * https://www.cnblogs.com/jwk000/p/3560086.html
 * Core Concept:
 * (1) time_point
 * (2) duration
 *      template <class Rep, class Period = ratio<1> > class duration;
 *        - Rep is something like int, double, float
 *        - Period describe splice of time, by second
 * (3) clock
 *
 *
 *
 */
void format_chrono() {
    auto now = std::chrono::system_clock::now();
    auto cnt = std::chrono::duration_cast<std::chrono::duration<int, std::ratio<1, 2>>>(100s);
    SK_LOG("cnt: {}", cnt.count());
}

int main() {
    format_chrono();
    return 0;
}
