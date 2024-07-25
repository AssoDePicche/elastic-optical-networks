#include "spectrum.h"

#include <algorithm>
#include <cassert>
#include <numeric>

Spectrum::Spectrum(const std::size_t size) : slots(std::vector(size, false)) {}

auto Spectrum::allocate(const std::size_t start, const std::size_t end)
    -> void {
  assert(end < size());

  assert(available_at(start, end));

  std::fill(slots.begin() + start, slots.begin() + end + 1, true);
}

auto Spectrum::deallocate(const std::size_t start, const std::size_t end)
    -> void {
  assert(end < size());

  assert(!available_at(start, end));

  std::fill(slots.begin() + start, slots.begin() + end + 1, false);
}

auto Spectrum::resize(const std::size_t size) -> void {
  slots = std::vector(size, false);
}

auto Spectrum::size(void) const noexcept -> std::size_t { return slots.size(); }

auto Spectrum::available(void) const noexcept -> std::size_t {
  return std::count(slots.begin(), slots.end(), false);
}

auto Spectrum::available_at(const std::size_t start,
                            const std::size_t end) const noexcept -> bool {
  return std::count(slots.begin() + start, slots.begin() + end + 1, true) == 0;
}

auto Spectrum::available_partitions(void) const noexcept
    -> std::vector<std::size_t> {
  auto length{0};

  auto in_free_block{false};

  std::vector<std::size_t> partitions{};

  for (const auto &slot : slots) {
    if (slot && in_free_block) {
      in_free_block = false;

      partitions.push_back(length);

      continue;
    }

    if (!slot) {
      if (!in_free_block) {
        in_free_block = true;

        length = 1;
      } else {
        ++length;
      }
    }
  }

  if (in_free_block) {
    partitions.push_back(length);
  }

  return partitions;
}

auto Spectrum::available_partitions(const std::size_t size) const noexcept
    -> std::vector<std::size_t> {
  const auto partitions = available_partitions();

  std::vector<size_t> suitable;

  std::copy_if(partitions.begin(), partitions.end(),
               std::back_inserter(suitable),
               [&](const std::size_t capacity) { return size <= capacity; });

  return suitable;
}

auto Spectrum::largest_partition(void) const noexcept -> std::size_t {
  const auto partitions = available_partitions();

  const auto ptr = std::max_element(partitions.begin(), partitions.end());

  return (ptr == partitions.end()) ? 0 : *ptr;
}

auto Spectrum::smallest_partition(void) const noexcept -> std::size_t {
  const auto partitions = available_partitions();

  const auto ptr = std::min_element(partitions.begin(), partitions.end());

  return (ptr == partitions.end()) ? 0 : *ptr;
}

auto Spectrum::fragmentation(void) const noexcept -> double {
  const auto total = static_cast<double>(available());

  if (total == 0.0) {
    return 0.0;
  }

  return (total - largest_partition()) / total;
}

auto Spectrum::fragmentation(const std::size_t size) const noexcept -> double {
  const auto total = static_cast<double>(available());

  if (total == 0.0) {
    return 0.0;
  }

  const auto partitions = available_partitions(size);

  const auto usable =
      std::accumulate(partitions.begin(), partitions.end(), 0.0);

  return (total - usable) / total;
}

auto Spectrum::to_string(void) const noexcept -> std::string {
  std::string buffer;

  std::for_each(slots.begin(), slots.end(), [&buffer](const bool slot) {
    buffer.append(slot ? "#" : ".");
  });

  return buffer;
}

auto first_fit(const Spectrum &spectrum, const std::size_t slots)
    -> std::optional<std::size_t> {
  assert(slots <= spectrum.size());

  if (slots > spectrum.largest_partition()) {
    return std::nullopt;
  }

  const auto size = spectrum.size();

  for (auto start{0u}; start < size; ++start) {
    auto fit = true;

    for (auto end{0u}; end < slots && (start + end) < size; ++end) {
      if (spectrum.slots[start + end]) {
        fit = false;

        break;
      }
    }

    if (!fit) {
      continue;
    }

    if (start + slots - 1 >= spectrum.size()) {
      return std::nullopt;
    }

    return start;
  }

  return std::nullopt;
}
