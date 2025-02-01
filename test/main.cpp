#include <gtest/gtest.h>

#include "graph.h"

TEST(Graph, DijkstraMinCost) {
  Graph graph(3);

  graph.add({0, 1, 2.0f});

  graph.add({1, 2, 2.0f});

  graph.add({0, 2, 5.0f});

  const auto expected_cost = 4.0f;

  const auto expected_hops = 1;

  const auto& [vertices, cost] = dijkstra(graph, 0, 2);

  // HOPS are vertices - 2 (we must exclude source and destination)

  const auto hops = vertices.size() - 2;

  ASSERT_EQ(cost, expected_cost);

  ASSERT_EQ(hops, expected_hops);
}

TEST(Graph, KShortestPath) {
  Graph graph(3);

  graph.add({0, 1, 2.0f});

  graph.add({1, 2, 2.0f});

  graph.add({0, 2, 4.0f});

  const auto k = 2u;

  const auto& k_paths = k_shortest_path(graph, 0, 2, k);

  const auto expected_cost = 8.0f;

  auto total_cost = Cost::min;

  for (const auto& [vertices, cost] : k_paths) {
    total_cost += cost;
  }

  ASSERT_EQ(k_paths.size(), k);

  ASSERT_EQ(total_cost, expected_cost);
}

auto main(int argc, char** argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
