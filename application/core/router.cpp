#include "router.h"

#include <hash/cantor.h>
#include <prng/prng.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

#include "yen.h"

namespace core {
struct Router::Implementation {
  Graph graph;
  std::unordered_map<uint64_t, std::vector<Route>> cache;

  Implementation(const Graph& graph) : graph{graph} {
    Yen yen(graph);
    for (Vertex source = 0; source < graph.size(); ++source) {
      for (Vertex destination = 0; destination < graph.size(); ++destination) {
        if (source == destination) continue;

        const auto key = hash::CantorPairingFunction(source, destination);

        const auto iterator = cache.find(key);

        if (iterator != cache.end()) {
          continue;
        }

        const std::vector<Path> paths = yen.compute(source, destination, 3);

        for (const auto& path : paths) {
          if (path.empty()) continue;

          const auto route =
              std::make_pair(std::unordered_set<Vertex>(path.vertices.begin(),
                                                        path.vertices.end()),
                             path.cost);

          cache[key].push_back(route);
        }
      }
    }

    std::shared_ptr<prng::PseudoRandomNumberGenerator> prng =
        prng::PseudoRandomNumberGenerator::Instance();

    prng->SetUniformVariable("routing", 0, cache.size() - 1);
  }

  Route compute(void) const {
    std::shared_ptr<prng::PseudoRandomNumberGenerator> prng =
        prng::PseudoRandomNumberGenerator::Instance();

    auto iterator = cache.begin();

    auto offset = static_cast<size_t>(std::floor(prng->Next("routing")));

    std::advance(iterator, std::min(offset, cache.size() - 1));

    return iterator->second[0];
  };

  std::optional<Route> compute(const Vertex source,
                               const Vertex destination) const {
    const auto key = hash::CantorPairingFunction(source, destination);

    const auto iterator = cache.find(key);

    if (iterator != cache.end()) {
      return iterator->second[0];
    }

    return std::nullopt;
  }
};

Router::Router(const Graph& graph)
    : pImpl(std::make_unique<Implementation>(graph)) {}

Router::~Router() = default;

Route Router::compute(void) const { return pImpl->compute(); }

std::optional<Route> Router::compute(const Vertex source,
                                     const Vertex destination) const {
  return pImpl->compute(source, destination);
}
}  // namespace core
