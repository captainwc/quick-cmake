#ifndef SHUAIKAI_DATASTRUCTURE_GRAPH_H
#define SHUAIKAI_DATASTRUCTURE_GRAPH_H

#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

#include "skutils/printer.h"
#include "skutils/random.h"

namespace sk::utils::dts {

namespace graphConst {
    const int dummyValue      = 0;
    const int wuxiangtuWeight = 1;
};  // namespace graphConst

template <typename ValueType>
class Graph {
public:
    std::vector<ValueType>        nodes;
    std::vector<std::vector<int>> edges;

    bool isYouXiang;
    int  nodeNum;

    Graph(bool isYouXiangTu = false) : isYouXiang(isYouXiangTu), nodeNum(0) {}

    Graph(std::vector<ValueType> v, bool isYouXiangTu = false)
        : nodes(std::move(v)),
          isYouXiang(isYouXiangTu),
          nodeNum(nodes.size()),
          edges(std::vector<std::vector<int>>(nodes.size(), std::vector<int>(nodes.size(), graphConst::dummyValue))) {}

    Graph(std::vector<ValueType> v, std::vector<std::vector<int>> e, bool isYouXiangTu = false)
        : nodes(std::move(v)), edges(std::move(e)), nodeNum(nodes.size()), isYouXiang(isYouXiangTu) {
        if (!isYouXiang) {
            for (int i = 0; i < nodeNum; i++) {
                for (int j = i + 1; j < nodeNum; j++) {
                    edges[i][j] = edges[j][i];
                }
            }
        }
    }

    int size() const { return nodeNum; }

    bool empty() const { return nodeNum == 0; }

    Graph<ValueType> &addNode(ValueType val) {
        nodes.emplace_back(std::move(val));
        ++nodeNum;
        edges.emplace_back(nodeNum, graphConst::dummyValue);
        for (int i = 0; i < nodeNum - 1; ++i) {
            edges[i].resize(nodeNum, graphConst::dummyValue);
        }
        return *this;
    }

    Graph<ValueType> &addEdge(int from, int to, int value = 1) {
        if (from >= nodeNum || to >= nodeNum) {
            throw std::out_of_range("node unexist");
        }
        if (from >= edges.size()) {
            throw std::out_of_range("edge auto expand false");
        }
        edges[from][to] = value;
        if (!isYouXiang) {
            edges[to][from] = value;
        }
        return *this;
    }

    std::string toString() const;
};

Graph<int> buildRandomGraph(int n = 10, bool isYouXiang = false, bool useDefaultWeight = true, int start = 1,
                            int end = 100) {
    std::vector<int>              vertex = RANDTOOL.getRandomIntVector(n, start, end);
    int                           dummy  = graphConst::dummyValue;
    std::vector<std::vector<int>> adjust(n, std::vector<int>(n, dummy));
    for (int i = 0; i < n * 3; i++) {
        std::vector<int> tup = RANDTOOL.getRandomIntVector(2, 0, n - 1);
        if (!useDefaultWeight) {
            adjust[tup[0]][tup[1]] = RANDTOOL.getRandomInt(start, end);
        } else {
            adjust[tup[0]][tup[1]] = graphConst::wuxiangtuWeight;
        }
    }
    if (!isYouXiang) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < i; j++) {
                if (adjust[i][j] != adjust[j][i]) {
                    if (RANDTOOL.coinOnce()) {
                        adjust[i][j] = adjust[j][i];
                    } else {
                        adjust[j][i] = adjust[i][j];
                    }
                }
            }
        }
    }
    return Graph<int>(vertex, adjust, isYouXiang);
}

template <typename ValueType>
std::string Graph<ValueType>::toString() const {
    std::stringstream ss;
    ss << "[nodes]:\n\t" << sk::utils::toString(nodes) << "\n";
    ss << "[edges]:\n";
    for (int i = 0; i < nodeNum; i++) {
        std::string tmp = sk::utils::format("{}", edges[i]);
        ss << "\t" << tmp << "\n";
    }
    std::string ret = ss.str();
    return ret;
}

template <>
std::string Graph<int>::toString() const {
    if (nodeNum == 0) {
        return "[EMPTY GRAPH]";
    }
    int max = 1;
    for (int i = 0; i < nodeNum; i++) {
        if (nodes[i] >= 10 && nodes[i] < 100) {
            max = 2;
        }
        if (nodes[i] >= 100) {
            max = 3;
        }
    }
    max++;

    std::stringstream sstmp;
    for (int i = 0; i < max * 2 + 3; i++) {
        sstmp << " ";
    }
    std::string prefixSpace = sstmp.str();

    std::stringstream ss;
    ss << prefixSpace;
    for (int i = 0; i < nodeNum; i++) {
        ss << std::setw(max) << nodes[i];
    }
    ss << "\n";
    ss << prefixSpace;
    for (int i = 0; i < nodeNum; i++) {
        ss << std::setw(max) << i;
    }
    ss << "\n";
    ss << prefixSpace;
    for (int i = 0; i < nodeNum; i++) {
        ss << std::setw(max) << "-";
    }
    ss << "\n";
    for (int i = 0; i < nodeNum; i++) {
        ss << "[" << std::setw(max) << nodes[i] << "]" << std::setw(max) << i << "|";
        for (int j = 0; j < nodeNum; ++j) {
            if (edges[i][j] == graphConst::dummyValue) {
                ss << std::setw(max) << " ";
            } else if (edges[i][j] == graphConst::wuxiangtuWeight) {
                // ss << setw(max) << "O";
                ss << std::setw(max) << edges[i][j];
            } else {
                ss << std::setw(max) << edges[i][j];
            }
        }
        ss << "\n";
    }
    std::string ret = ss.str();
    return ret;
}

}  // namespace sk::utils::dts

#endif  // SHUAIKAI_DATASTRUCTURE_GRAPH_H
