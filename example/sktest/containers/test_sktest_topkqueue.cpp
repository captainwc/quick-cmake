#include <vector>

#include "skutils/containers/heap.h"
#include "skutils/containers/topk_queue.h"
#include "skutils/test.h"

int main() {
    std::vector<int> vc{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    sk::utils::dts::topbottomk_queue<int> tbq(5);
    sk::utils::dts::topk_queue<int>       tq(5);

    for (auto x : vc) {
        tq.push(x);
        tbq.push(x);
    }

    auto correct_top_k    = std::vector<int>{15, 14, 13, 12, 11};
    auto correct_bottom_k = std::vector<int>{1, 2, 3, 4, 5};

    ASSERT_STR_EQUAL(correct_top_k, tq.pop());
    ASSERT_STR_EQUAL(correct_top_k, tbq.pop_top());
    ASSERT_STR_EQUAL(correct_bottom_k, tbq.pop_bottom());

    return ASSERT_ALL_PASSED();
}