#include "router.h"

#include <hash/cantor.h>
#include <prng/prng.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <unordered_map>
#include <vector>

#include "yen.h"

namespace core {
struct Router::Implementation {
  Graph graph;
  std::unordered_map<uint64_t, std::vector<Path>> cache;

  Implementation(const Graph& graph) : graph{graph} {
    Yen yen(graph);

    for (Graph::Vertex source = 0; source < graph.size(); ++source) {
      for (Graph::Vertex destination = 0; destination < graph.size();
           ++destination) {
        if (source == destination) continue;

        const std::vector<Path> paths = yen.compute(source, destination, 3);

        const auto key = hash::CantorPairingFunction(source, destination);

        for (const auto& path : paths) {
          if (path.empty()) continue;

          cache[key].push_back(path);
        }
      }
    }

    std::shared_ptr<prng::PseudoRandomNumberGenerator> prng =
        prng::PseudoRandomNumberGenerator::Instance();

    prng->SetUniformVariable("routing", 0, cache.size() - 1);
  }

  Path compute(void) const {
    std::shared_ptr<prng::PseudoRandomNumberGenerator> prng =
        prng::PseudoRandomNumberGenerator::Instance();

    auto iterator = cache.begin();

    auto offset = static_cast<size_t>(std::floor(prng->Next("routing")));

    std::advance(iterator, std::min(offset, cache.size() - 1));

    return iterator->second[0];
  };
};

Router::Router(const Graph& graph)
    : pImpl(std::make_unique<Implementation>(graph)) {}

Router::~Router() = default;

Path Router::compute(void) const { return pImpl->compute(); }
}  // namespace core
