#include <string>

#include "skutils/containers/graph.h"

namespace ks = sk::utils;

int main() {
    auto g = ks::dts::buildRandomGraph();
    std::cout << g.toString();
}
