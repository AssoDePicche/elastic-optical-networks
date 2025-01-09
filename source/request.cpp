#include "request.h"

#include <limits>

Request::Request(const route_t &route, const unsigned bandwidth)
    : route{route}, bandwidth{bandwidth} {}

auto from_gigabits_transmission(const double distance) -> unsigned {
  if (distance <= 160.0) {
    return 5u;
  }

  if (distance <= 880.0) {
    return 6u;
  }

  if (distance <= 2480.0) {
    return 7u;
  }

  if (distance <= 3120.0) {
    return 9u;
  }

  if (distance <= 5000) {
    return 10u;
  }

  if (distance <= 6080.0) {
    return 12u;
  }

  if (distance <= 8000.0) {
    return 13u;
  }

  return std::numeric_limits<unsigned>::max();
}

auto from_terabits_transmission(const double distance) -> unsigned {
  if (distance <= 400.0) {
    return 14u;
  }

  if (distance <= 800.0) {
    return 15u;
  }

  if (distance <= 1600.0) {
    return 17u;
  }

  if (distance <= 3040.0) {
    return 19u;
  }

  if (distance <= 4160.0) {
    return 22u;
  }

  if (distance <= 6400.0) {
    return 25u;
  }

  if (distance <= 8000.0) {
    return 28u;
  }

  return std::numeric_limits<unsigned>::max();
}

auto make_key(unsigned x, unsigned y) -> unsigned {
  return ((x + y) * (x + y + 1) / 2) + y;
}

auto route_keys(const route_t &route) -> std::vector<unsigned> {
  const auto &[vertices, cost] = route;

  assert(!vertices.empty());

  std::vector<unsigned> keys;

  std::vector<vertex_t> v;

  for (const auto &vertex : vertices) {
    v.push_back(vertex);
  }

  for (auto index{1u}; index < v.size(); ++index) {
    keys.push_back(make_key(v[index - 1], v[index]));
  }

  return keys;
}

auto make_hashmap(const Graph &graph,
                  const unsigned size) -> std::map<unsigned, Spectrum> {
  std::map<unsigned, Spectrum> hashmap;

  const auto edges = graph.get_edges();

  for (const auto &[source, destination, cost] : edges) {
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
