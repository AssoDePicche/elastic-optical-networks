#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "graph.h"

struct Route final {
  std::unordered_set<Vertex> vertices;
  Cost cost;

  [[nodiscard]] static Route None(void);
};

class RoutingStrategy {
 public:
  RoutingStrategy(const Graph &);

  virtual ~RoutingStrategy() = default;

  [[nodiscard]] virtual Route compute(const Vertex, const Vertex) const = 0;

 protected:
  const Graph &graph;
};

class BreadthFirstSearch : public RoutingStrategy {
 public:
  BreadthFirstSearch(const Graph &);

  [[nodiscard]] Route compute(const Vertex, const Vertex) const override;
};

class DepthFirstSearch : public RoutingStrategy {
 public:
  DepthFirstSearch(const Graph &);

  [[nodiscard]] Route compute(const Vertex, const Vertex) const override;
};

class Dijkstra : public RoutingStrategy {
 public:
  Dijkstra(const Graph &);

  [[nodiscard]] Route compute(const Vertex, const Vertex) const override;
};

class RandomRouting : public RoutingStrategy {
 public:
  RandomRouting(const Graph &);

  [[nodiscard]] Route compute(const Vertex, const Vertex) const override;
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

  [[nodiscard]] Route compute(const Vertex, const Vertex);

 private:
  std::unordered_map<unsigned, Route> cache;
  std::shared_ptr<RoutingStrategy> strategy;
};
