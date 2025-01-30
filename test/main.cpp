#include <gtest/gtest.h>

#include "graph.h"

TEST(Graph, DijkstraMinCost) {
  Graph graph(3);

  graph.add({0, 1, 2.0f});

  graph.add({1, 2, 2.0f});

  graph.add({0, 2, 5.0f});

  const auto& [vertices, cost] = dijkstra(graph, 0, 2);

  ASSERT_EQ(cost, 4.0f);
}

TEST(Graph, DijkstraMinHopsCost) {
  Graph graph(3);

  graph.add({0, 1, 2.0f});

  graph.add({1, 2, 2.0f});

  graph.add({0, 2, 4.0f});

  const auto& [vertices, cost] = dijkstra(graph, 0, 2);

  // HOPS are vertices - 2 (we must exclude source and destination)

  ASSERT_EQ(vertices.size() - 2, 1);
}

auto main(int argc, char** argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
