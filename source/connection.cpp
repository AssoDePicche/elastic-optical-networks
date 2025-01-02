#include "connection.h"

#include <limits>

Connection::Connection(const Path &path, const unsigned slots)
    : path{path}, slots{slots} {}

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

auto path_keys(const Path &path) -> std::vector<unsigned> {
  assert(!path.vertices.empty());

  std::vector<unsigned> keys;

  for (auto index{1u}; index < path.vertices.size(); ++index) {
    keys.push_back(make_key(path.vertices[index - 1], path.vertices[index]));
  }

  return keys;
}

auto make_hashmap(const Graph &graph,
                  const unsigned size) -> std::map<unsigned, Spectrum> {
  std::map<unsigned, Spectrum> hashmap;

  for (auto source{0u}; source < graph.size(); ++source) {
    for (auto destination{0u}; destination < graph.size(); ++destination) {
      if (!graph.adjacent(source, destination)) {
        continue;
      }

      const auto key{make_key(source, destination)};

      hashmap[key] = Spectrum(size);
    }
  }

  return hashmap;
}

auto make_connection(Connection &connection,
                     std::map<unsigned, Spectrum> &hashmap,
                     const SpectrumAllocator &spectrum_allocator) -> bool {
  const auto keys{path_keys(connection.path)};

  const auto search{spectrum_allocator(hashmap[keys[0]], connection.slots)};

  if (!search.has_value()) {
    return false;
  }

  connection.slice = search.value();

  for (const auto &key : keys) {
    if (!hashmap[key].available_at(connection.slice)) {
      return false;
    }
  }

  for (const auto &key : keys) {
    hashmap[key].allocate(connection.slice);
  }

  return true;
}
