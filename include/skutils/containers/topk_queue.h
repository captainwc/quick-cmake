#ifndef SK_DATASTRUCTURE_TOP_K_QUEUE
#define SK_DATASTRUCTURE_TOP_K_QUEUE

#include <algorithm>
#include <mutex>
#include <queue>
#include <vector>

#include "skutils/printer.h"
#include "skutils/random.h"
#include "skutils/spinlock.h"

namespace sk::utils::dts {

template <typename T, typename Comp>
struct reverse_comp {
    struct reversed_type {
        bool operator()(const T& a, const T& b) const {
            auto cmp = Comp();
            return (!cmp(a, b)) && (!(!cmp(a, b) && !cmp(b, a)));
        }
    };
};

template <typename T, typename Comp = std::less<T>>
class topk_queue {
private:
    size_t cap{};

    using ReversedComp = typename reverse_comp<T, Comp>::reversed_type;
    std::priority_queue<T, std::vector<T>, ReversedComp> pq;

    sk::utils::SpinLock spinlock;

public:
    explicit topk_queue(size_t capacity) : cap(capacity) {}

    topk_queue(const topk_queue& q) : cap(q.cap), pq(q.pq) {}

    topk_queue(topk_queue&& o) noexcept : cap(o.cap), pq(std::move(o.pq)) {}

    topk_queue& operator=(const topk_queue& o) {
        if (&o != this) {
            this->cap = pq.cap;
            this->pq  = o.pq;
        }
        return *this;
    }

    topk_queue& operator=(topk_queue&& o) noexcept {
        this->cap = o.cap;
        this->pq  = o.pq;
        return *this;
    }

    ~topk_queue() = default;

    void push(const T& val) {
        sk::utils::SpinLockGuard guard{spinlock};
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
        {
            sk::utils::SpinLockGuard guard{spinlock};
            while (!pq.empty()) {
                ret.emplace_back(pq.top());
                pq.pop();
            }
        }
        std::reverse(ret.begin(), ret.end());
        return ret;
    }
};

template <typename T, typename Comp = std::less<>>
class topbottomk_queue {
private:
    topk_queue<T, Comp>                                          max_queue;
    topk_queue<T, typename reverse_comp<T, Comp>::reversed_type> min_queue;

public:
    explicit topbottomk_queue(size_t capacity) : max_queue(capacity), min_queue(capacity) {}

    topbottomk_queue(const topbottomk_queue& o) : max_queue(o.max_queue), min_queue(o.min_queue) {}

    topbottomk_queue& operator=(const topbottomk_queue& o) {
        if (&o != this) {
            this->max_queue = o.max_queue;
            this->min_queue = o.min_queue;
        }
        return *this;
    }

    topbottomk_queue(topbottomk_queue&& o) noexcept
        : max_queue(std::move(o.max_queue)), min_queue(std::move(o.min_queue)) {}

    topbottomk_queue& operator=(topbottomk_queue&& o) noexcept {
        this->max_queue = o.max_queue;
        this->min_queue = o.min_queue;
        return *this;
    }

    ~topbottomk_queue() = default;

    void push(const T& val) {
        max_queue.push(val);
        min_queue.push(val);
    }

    std::vector<T> pop_top() { return max_queue.pop(); }

    std::vector<T> pop_bottom() { return min_queue.pop(); }
};

}  // namespace sk::utils::dts

#endif