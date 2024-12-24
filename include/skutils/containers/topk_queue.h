#ifndef SK_DATASTRUCTURE_TOP_K_QUEUE
#define SK_DATASTRUCTURE_TOP_K_QUEUE

#include <algorithm>
#include <queue>
#include <vector>

namespace sk::utils::dts {

template <typename T, typename Comp>
struct reverse_comp {
    struct type {
        bool operator()(const T& a, const T& b) const {
            auto cmp = Comp();
            return (!cmp(a, b)) && (!(!cmp(a, b) && !cmp(b, a)));
        }
    };
};

template <typename T, typename Comp = std::less<T>>
class topk_queue {
private:
    size_t cap;

    using ReversedComp = typename reverse_comp<T, Comp>::type;
    std::priority_queue<T, std::vector<T>, ReversedComp> pq;

public:
    explicit topk_queue(size_t capacity) : cap(capacity) {}

    void push(const T& val) {
        if (pq.size() < cap) {
            pq.push(val);
        } else if (ReversedComp()(val, pq.top())) {
            pq.push(val);
            pq.pop();
        } else {
            // do nothing
        }
    }

    std::vector<T> pop() {
        std::vector<T> ret;
        while (!pq.empty()) {
            ret.emplace_back(pq.top());
            pq.pop();
        }
        std::reverse(ret.begin(), ret.end());
        return ret;
    }
};

template <typename T, typename Comp = std::less<>>
class topbottomk_queue {
private:
    topk_queue<T, Comp>                                 max_queue;
    topk_queue<T, typename reverse_comp<T, Comp>::type> min_queue;

public:
    explicit topbottomk_queue(size_t capacity) : max_queue(capacity), min_queue(capacity) {}

    void push(const T& val) {
        max_queue.push(val);
        min_queue.push(val);
    }

    std::vector<T> pop_top() {
        return max_queue.pop();
    }

    std::vector<T> pop_bottom() {
        return min_queue.pop();
    }
};

}  // namespace sk::utils::dts

#endif