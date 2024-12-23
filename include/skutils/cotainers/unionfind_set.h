#ifndef SHUAIKAI_DATASTRUCTURE_UNION_FIND_SET_H
#define SHUAIKAI_DATASTRUCTURE_UNION_FIND_SET_H

#include <map>

#include "graph.h"

namespace sk::utils::dts {

template <typename T, typename Less = std::less<T>>
class UnionFindSet {
private:
    int                  cnt;
    std::map<T, T, Less> parent;
    Less                 lesser;

    bool equal(const T &a, const T &b) const {
        return !lesser(a, b) && !lesser(b, a);
    }

public:
    UnionFindSet() : cnt(0), lesser(Less()) {}

    explicit UnionFindSet(Graph<T> g) : cnt(g.size()) {
        for (auto val : g.nodes) {
            parent.emplace(val, val);
        }
        for (int i = 0; i < g.size(); i++) {
            if (g.isYouXiang) {
                for (int j = 0; j < g.size(); j++) {
                    if (g.edges[i][j] != graphConst::dummyValue) {
                        connect(g.nodes[i], g.nodes[j]);
                    }
                }
            } else {
                for (int j = 0; j < i; j++) {
                    if (g.edges[i][j] != graphConst::dummyValue) {
                        connect(g.nodes[i], g.nodes[j]);
                    }
                }
            }
        }
    }

    int count() const {
        return cnt;
    }

    void add(T val) {
        auto [it, yes] = parent.emplace(val, val);
        if (yes) {
            ++cnt;
        }
    }

    void add(T key, T val) {
        auto [it1, yes1] = parent.emplace(key, key);
        if (yes1) {
            ++cnt;
        }
        auto [it2, yes2] = parent.emplace(val, val);
        if (yes2) {
            ++cnt;
        }
        connect(key, val);
    }

    T find(T key) {
        if (parent.find(key) == parent.end()) {
            throw std::out_of_range(formatStr("key {} Unexist.", key));
        }
        T p = parent[key];
        if (equal(p, key)) {  // equal要求const参数，传递parent[key]会导致map类型变为const map，进而无法引用
            return key;
        }
        return find(p);
    }

    void connect(T key1, T key2) {
        T r1 = find(key1);
        T r2 = find(key2);
        if (!equal(r1, r2)) {
            parent[r1] = r2;
            --cnt;
        }
    }

    std::string toString() const {
        if (cnt == 0) {
            return std::string{"[]"};
        }
        std::vector<T> top;
        for (auto [k, v] : parent) {
            if (equal(k, v)) {
                top.emplace_back(k);
            }
        }
        std::stringstream ss;
        for (auto n : top) {
            ss << toStringHelper(n) << "\n";
        }
        std::string ret = ss.str();
        return ret;
    }

    std::string toStringHelper(T a) const {
        std::stringstream ss;
        ss << formatStr("{}", a) << "-(";
        for (auto [k, v] : parent) {
            if (equal(v, a) && !equal(k, a)) {
                ss << toStringHelper(k) << ",";
            }
        }
        std::string ret = ss.str();
        if (ret.back() == ',') {
            ret.pop_back();
            ret += ")";
        } else {
            ret.pop_back();
            ret.pop_back();
        }
        return ret;
    }
};

template <>
class UnionFindSet<int> {
private:
    int              cnt;
    std::vector<int> parent;

public:
    explicit UnionFindSet(int n) : cnt(n), parent(n) {
        for (int i = 0; i < n; i++) {
            parent[i] = i;
        }
    }

    explicit UnionFindSet(Graph<int> g) : cnt(g.size()), parent(cnt) {
        for (int i = 0; i < cnt; i++) {
            parent[i] = i;
        }
        int len = g.size();
        for (int i = 0; i < len; i++) {
            if (g.isYouXiang) {
                for (int j = 0; j < len; j++) {
                    if (g.edges[i][j] != graphConst::dummyValue) {
                        connect(i, j);
                    }
                }
            } else {
                for (int j = 0; j < i; j++) {
                    if (g.edges[i][j] != graphConst::dummyValue) {
                        // LOGV("connect {}<->{}", i, j);
                        connect(i, j);
                    }
                }
            }
        }
    }

    int count() const {
        return cnt;
    }

    int find(int a) {
        // 标准的采用路径压缩算法，以尽量减少树的高度
        if (parent[a] != a) {
            parent[a] = find(parent[a]);
        }
        return parent[a];
    }

    UnionFindSet &connect(int a, int b) {
        int ra = find(a);
        int rb = find(b);
        if (ra != rb) {
            parent[rb] = ra;
            cnt--;
        }
        return *this;
    }

    void normalize() {
        for (int i = 0; i < parent.size(); i++) {
            if (parent[i] != i) {
                parent[i] = find(parent[i]);
            }
        }
    }

    bool isConnected(int a, int b) {
        return find(a) == find(b);
    }

    std::string toString() const {
        std::vector<int> top;
        for (int i = 0; i < parent.size(); i++) {
            if (parent[i] == i) {
                top.push_back(i);
            }
        }
        std::stringstream ss;
        for (auto n : top) {
            ss << toStringHelper(n) << "\n";
        }
        std::string ret = ss.str();
        return ret;
    }

    std::string toStringHelper(int a) const {
        std::stringstream ss;
        ss << a << "-(";
        for (int i = 0; i < parent.size(); i++) {
            if (parent[i] == a && i != a) {
                ss << toStringHelper(i) << ",";
            }
        }
        std::string ret = ss.str();
        if (ret.back() == ',') {
            ret.pop_back();
            ret += ")";
        } else {
            ret.pop_back();
            ret.pop_back();
        }
        return ret;
    }
};

}  // namespace sk::utils::dts

#endif  // SHUAIKAI_DATASTRUCTURE_UNION_FIND_SET_H
