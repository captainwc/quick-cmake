#include <iostream>
#include <string>

#include "skutils/containers/graph.h"

namespace ks = sk::utils;

int main() {
  // Test 1: Create a directed graph and add nodes and edges
  std::cout << "=== Test 1: Directed Graph Basic Operations ===" << std::endl;
  ks::dts::Graph<std::string> directedGraph(true);

  directedGraph.addNode("A");
  directedGraph.addNode("B");
  directedGraph.addNode("C");
  directedGraph.addNode("D");

  directedGraph.addEdge(0, 1, 5);  // A -> B, weight=5
  directedGraph.addEdge(1, 2, 3);  // B -> C, weight=3
  directedGraph.addEdge(0, 3, 2);  // A -> D, weight=2
  directedGraph.addEdge(3, 2, 4);  // D -> C, weight=4

  std::cout << directedGraph.toString() << std::endl;

  // Test 2: Create an undirected graph and add nodes and edges
  std::cout << "=== Test 2: Undirected Graph Basic Operations ===" << std::endl;
  ks::dts::Graph<std::string> undirectedGraph(false);

  undirectedGraph.addNode("X");
  undirectedGraph.addNode("Y");
  undirectedGraph.addNode("Z");

  undirectedGraph.addEdge(0, 1, 10);  // X - Y, weight=10
  undirectedGraph.addEdge(1, 2, 7);   // Y - Z, weight=7
  undirectedGraph.addEdge(0, 2, 15);  // X - Z, weight=15

  std::cout << undirectedGraph.toString() << std::endl;

  // Test 3: Create graph with predefined nodes
  std::cout << "=== Test 3: Graph with Predefined Nodes ===" << std::endl;
  std::vector<std::string> nodes = {"Node1", "Node2", "Node3", "Node4"};
  ks::dts::Graph<std::string> graphWithNodes(nodes, true);

  graphWithNodes.addEdge(0, 1);
  graphWithNodes.addEdge(1, 2);
  graphWithNodes.addEdge(2, 3);
  graphWithNodes.addEdge(3, 0);

  std::cout << graphWithNodes.toString() << std::endl;

  // Test 4: Use random graph generator (directed graph)
  std::cout << "=== Test 4: Random Directed Graph ===" << std::endl;
  auto randomDirectedGraph = ks::dts::buildRandomGraph(6, true, false, 1, 50);
  std::cout << randomDirectedGraph.toString() << std::endl;

  // Test 5: Use random graph generator (undirected graph with default weights)
  std::cout << "=== Test 5: Random Undirected Graph (Default Weights) ===" << std::endl;
  auto randomUndirectedGraph = ks::dts::buildRandomGraph(5, false, true, 1, 100);
  std::cout << randomUndirectedGraph.toString() << std::endl;

  // Test 6: Edge case - Empty graph
  std::cout << "=== Test 6: Empty Graph ===" << std::endl;
  ks::dts::Graph<int> emptyGraph;
  std::cout << emptyGraph.toString() << std::endl;

  // Test 7: Graph with integer nodes
  std::cout << "=== Test 7: Graph with Integer Nodes ===" << std::endl;
  ks::dts::Graph<int> intGraph;
  intGraph.addNode(10);
  intGraph.addNode(20);
  intGraph.addNode(30);
  intGraph.addEdge(0, 1, 100);
  intGraph.addEdge(1, 2, 200);
  std::cout << intGraph.toString() << std::endl;

  return 0;
}
