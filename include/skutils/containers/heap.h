#ifndef SHUAIKAI_DATASTRUCTURE_HEAP_H
#define SHUAIKAI_DATASTRUCTURE_HEAP_H

#include <string>
#include <vector>

#include "skutils/printer.h"

namespace sk::utils::dts {

template <typename ValueType, typename Compare = std::greater<>>
class Heap {
private:
    Compare                comp;
    std::vector<ValueType> data;
    int                    elemNums;

    void buildHeap();
    void adjustUp(int k);
    void adjustDown(int k);

public:
    Heap() : comp(Compare()), data(1, ValueType{}), elemNums(0) {}

    explicit Heap(std::vector<ValueType> vals) : comp(Compare()), data(std::move(vals)), elemNums(data.size()) {
        data.insert(data.begin(), ValueType{});
        buildHeap();
    }

    explicit Heap(const Compare &cmp) : comp(cmp), data(1, ValueType{}), elemNums(0) {}

    ValueType &top();

    void pop() noexcept;
    void push(ValueType val);

    bool empty() const {
        return elemNums == 0;
    }

    int size() const {
        return elemNums;
    }

    std::string toString() const {
        return sk::utils::toString(std::vector<ValueType>(data.begin() + 1, data.begin() + elemNums));
    }

    static void sort(std::vector<ValueType> &vc);
};

template <typename ValueType, typename Compare>
void Heap<ValueType, Compare>::buildHeap() {
    for (int i = elemNums / 2; i >= 1; --i) {
        adjustDown(i);
    }
}

template <typename ValueType, typename Compare>
void Heap<ValueType, Compare>::adjustDown(int k) {
    while (2 * k <= elemNums) {
        int j = 2 * k;
        if (j < elemNums && comp(data[j], data[j + 1])) {
            j++;
        }
        if (!comp(data[k], data[j])) {
            break;
        }
        std::swap(data[k], data[j]);
        k = j;
    }
}

template <typename ValueType, typename Compare>
void Heap<ValueType, Compare>::adjustUp(int k) {
    while (k > 1 && comp(data[k], data[k / 2])) {
        std::swap(data[k], data[k / 2]);
        k /= 2;
    }
}

template <typename ValueType, typename Compare>
ValueType &Heap<ValueType, Compare>::top() {
    if (elemNums >= 1) {
        return data[1];
    }
    throw std::out_of_range("Heap Empty");
}

template <typename ValueType, typename Compare>
void Heap<ValueType, Compare>::pop() noexcept {
    if (elemNums > 0) {
        std::swap(data[1], data[elemNums]);
        --elemNums;
        adjustDown(1);
    }
}

template <typename ValueType, typename Compare>
void Heap<ValueType, Compare>::push(ValueType val) {
    ++elemNums;
    data.resize(elemNums + 1);
    data[elemNums] = std::move(val);
    adjustUp(elemNums);
}

template <typename ValueType, typename Compare>
void Heap<ValueType, Compare>::sort(std::vector<ValueType> &vc) {
    std::vector<ValueType> ret;
    auto                   heap = Heap<ValueType, Compare>(vc);
    while (!heap.empty()) {
        ret.push_back(heap.top());
        heap.pop();
    }
    vc.assign(ret.begin(), ret.end());
}

}  //  namespace sk::utils::dts

#endif  // SHUAIKAI_DATASTRUCTURE_HEAP_H
