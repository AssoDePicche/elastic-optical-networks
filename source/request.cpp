#include "request.h"

#include <limits>
#include <ranges>

Request::Request(const route_t &route, const bandwidth_t bandwidth)
    : route{route}, bandwidth{bandwidth} {}

bandwidth_t from_gigabits_transmission(const cost_t cost) noexcept {
  if (cost <= 160.0f) {
    return 5u;
  }

  if (cost <= 880.0f) {
    return 6u;
  }

  if (cost <= 2480.0f) {
    return 7u;
  }

  if (cost <= 3120.0f) {
    return 9u;
  }

  if (cost <= 5000.0f) {
    return 10u;
  }

  if (cost <= 6080.0f) {
    return 12u;
  }

  if (cost <= 8000.0f) {
    return 13u;
  }

  return max_bandwidth;
}

bandwidth_t from_terabits_transmission(const cost_t cost) noexcept {
  if (cost <= 400.0f) {
    return 14u;
  }

  if (cost <= 800.0f) {
    return 15u;
  }

  if (cost <= 1600.0f) {
    return 17u;
  }

  if (cost <= 3040.0f) {
    return 19u;
  }

  if (cost <= 4160.0f) {
    return 22u;
  }

  if (cost <= 6400.0f) {
    return 25u;
  }

  if (cost <= 8000.0f) {
    return 28u;
  }

  return max_bandwidth;
}

auto make_key(unsigned x, unsigned y) -> unsigned {
  return ((x + y) * (x + y + 1) / 2) + y;
}

auto route_keys(const route_t &route) -> std::vector<unsigned> {
  assert(!route.vertices.empty());

  std::vector<vertex_t> vertices(route.vertices.begin(), route.vertices.end());

  std::vector<unsigned> keys;

  keys.reserve(vertices.size() - 1u);

  for (const auto &index : std::views::iota(1u, vertices.size())) {
    const auto x = vertices.at(index - 1);

    const auto y = vertices.at(index);

    keys.emplace_back(make_key(x, y));
  }

  return keys;
}

auto make_hashmap(const Graph &graph,
                  const unsigned size) -> std::map<unsigned, Spectrum> {
  std::map<unsigned, Spectrum> hashmap;

  for (const auto &[source, destination, cost] : graph.get_edges()) {
    const auto key = make_key(source, destination);

    hashmap[key] = Spectrum(size);
  }

  return hashmap;
}

auto dispatch_request(Request &request, std::map<unsigned, Spectrum> &hashmap,
                      const SpectrumAllocator &spectrum_allocator) -> bool {
  const auto keys{route_keys(request.route)};

  const auto search{spectrum_allocator(hashmap[keys[0]], request.bandwidth)};

  if (!search.has_value()) {
    return false;
  }

  request.slice = search.value();

  for (const auto &key : keys) {
    if (!hashmap[key].available_at(request.slice)) {
      return false;
    }
  }

  for (const auto &key : keys) {
    hashmap[key].allocate(request.slice);
  }

  return true;
}
