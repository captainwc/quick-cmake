#ifndef SK_DATASTRUCTURE_TOP_K_QUEUE
#define SK_DATASTRUCTURE_TOP_K_QUEUE

#include <queue>

namespace sk::utils::dts {

template <typename T, typename Comp = std::less<T>>
class topk_queue {
private:
    std::priority_queue<T, std::vector<T>, Comp> _q;

    size_t sz;
    size_t cap;

public:
    using value_type = T;

    explicit topk_queue(size_t capacity) : cap(capacity), sz(0){};

    const T& top() const;

    bool empty() const;

    size_t size() const;

    size_t capacity() const;

    void pop();

    void push(const T& elem);

    void push(T&&);
};

}  //  namespace sk::utils::dts

#endif