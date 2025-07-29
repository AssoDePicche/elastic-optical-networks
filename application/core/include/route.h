#pragma once

#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "graph.h"

using Route = std::pair<std::unordered_set<Vertex>, Cost>;

class RoutingStrategy {
 public:
  RoutingStrategy(const Graph &);

  virtual ~RoutingStrategy() = default;

  [[nodiscard]] virtual std::optional<Route> compute(const Vertex,
                                                     const Vertex) const = 0;

 protected:
  const Graph &graph;
};

class BreadthFirstSearch : public RoutingStrategy {
 public:
  BreadthFirstSearch(const Graph &);

  [[nodiscard]] std::optional<Route> compute(const Vertex,
                                             const Vertex) const override;
};

class DepthFirstSearch : public RoutingStrategy {
 public:
  DepthFirstSearch(const Graph &);

  [[nodiscard]] std::optional<Route> compute(const Vertex,
                                             const Vertex) const override;
};

class Dijkstra : public RoutingStrategy {
 public:
  Dijkstra(const Graph &);

  [[nodiscard]] std::optional<Route> compute(const Vertex,
                                             const Vertex) const override;
};

class RandomRouting : public RoutingStrategy {
 public:
  RandomRouting(const Graph &);

  [[nodiscard]] std::optional<Route> compute(const Vertex,
                                             const Vertex) const override;
};

class KShortestPath {
 public:
  KShortestPath(const Graph &);

  [[nodiscard]] std::vector<Route> compute(const Vertex, const Vertex,
                                           const unsigned) const;

 private:
  const Graph &graph;
};

class Router final {
 public:
  void SetStrategy(std::shared_ptr<RoutingStrategy>);

  [[nodiscard]] std::optional<Route> compute(const Vertex, const Vertex);

 private:
  std::unordered_map<unsigned, Route> cache;
  std::shared_ptr<RoutingStrategy> strategy;
};
