#include "agent.h"

#include <graph/router.h>
#include <hash/cantor.h>

#include <ranges>
#include <unordered_set>

namespace core {
struct ClassicAgent::Implementation {
  bool ShouldAccept(Environment& environment) {
    if (environment.FSUsPerLink <= environment.activeRequests) {
      return false;
    }

    const auto keys = GenerateKeys(environment.request.route);

    const auto first = *keys.begin();

    const auto slice = environment.request.type.allocator(
        environment.carriers[first], environment.request.type.FSUs);

    if (!slice.has_value()) {
      return false;
    }

    environment.request.slice = slice.value();

    for (const auto& key : keys) {
      if (environment.carriers.at(key).available() <
              environment.request.type.FSUs ||
          !environment.carriers.at(key).available_at(
              environment.request.slice)) {
        return false;
      }
    }

    return true;
  }

  uint64_t GenerateKeys(const graph::Vertex source,
                        const graph::Vertex destination) const {
    return hash::CantorPairingFunction(source, destination);
  }

  std::unordered_set<uint64_t> GenerateKeys(const graph::Route& route) const {
    const auto& [vertices, cost] = route;

    std::unordered_set<uint64_t> keys;

    for (const auto& index : std::views::iota(1u, vertices.size())) {
      const auto x = *std::next(vertices.begin(), index - 1);

      const auto y = *std::next(vertices.begin(), index);

      keys.insert(hash::CantorPairingFunction(x, y));
    }

    return keys;
  }
};

ClassicAgent::ClassicAgent() { pImpl = std::make_unique<Implementation>(); }

ClassicAgent::~ClassicAgent() {}

bool ClassicAgent::ShouldAccept(Environment& environment) {
  return pImpl->ShouldAccept(environment);
}
}  // namespace core
