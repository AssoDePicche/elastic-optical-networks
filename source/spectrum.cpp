#include "spectrum.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>
#include <random>

#include "distribution.h"

Spectrum::Spectrum(const unsigned size) : slots(std::vector(size, false)) {}

auto Spectrum::allocate(const Slice &slice) -> void {
  const auto &[start, end] = slice;

  assert(end < size());

  assert(available_at(slice));

  std::fill(slots.begin() + start, slots.begin() + end + 1, true);
}

auto Spectrum::deallocate(const Slice &slice) -> void {
  const auto &[start, end] = slice;

  assert(end < size());

  assert(!available_at(slice));

  std::fill(slots.begin() + start, slots.begin() + end + 1, false);
}

auto Spectrum::resize(const unsigned size) -> void {
  slots = std::vector(size, false);
}

auto Spectrum::size(void) const noexcept -> unsigned { return slots.size(); }

auto Spectrum::available(void) const noexcept -> unsigned {
  return std::count(slots.begin(), slots.end(), false);
}

auto Spectrum::available_at(const Slice &slice) const noexcept -> bool {
  const auto &[start, end] = slice;

  return std::count(slots.begin() + start, slots.begin() + end + 1, true) == 0;
}

auto Spectrum::available_partitions(void) const noexcept
    -> std::vector<unsigned> {
  auto length{0};

  auto in_free_block{false};

  std::vector<unsigned> partitions{};

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

auto Spectrum::available_partitions(const unsigned size) const noexcept
    -> std::vector<unsigned> {
  const auto partitions = available_partitions();

  std::vector<unsigned> suitable;

  std::copy_if(partitions.begin(), partitions.end(),
               std::back_inserter(suitable),
               [&](const unsigned capacity) { return size <= capacity; });

  return suitable;
}

auto Spectrum::largest_partition(void) const noexcept -> unsigned {
  const auto partitions = available_partitions();

  const auto ptr = std::max_element(partitions.begin(), partitions.end());

  return (ptr == partitions.end()) ? 0 : *ptr;
}

auto Spectrum::smallest_partition(void) const noexcept -> unsigned {
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

auto Spectrum::fragmentation(const unsigned size) const noexcept -> double {
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

auto Spectrum::at(const unsigned index) const -> bool {
  assert(index < size());

  return slots[index];
}

auto Spectrum::gaps(void) const -> unsigned {
  auto gaps = 0u;

  auto in_gap = false;

  for (const auto &slot : slots) {
    if (!slot && in_gap) {
      continue;
    }

    if (!slot && !in_gap) {
      in_gap = true;
    }

    if (slot) {
      in_gap = false;
    }

    if (in_gap) {
      ++gaps;
    }
  }

  return gaps;
}

auto Spectrum::largest_gap(void) const -> unsigned {
  auto largest = 0u;

  auto in_gap = false;

  auto gap_size = 0u;

  for (const auto &slot : slots) {
    if (!slot && in_gap) {
      ++gap_size;

      continue;
    }

    if (!slot && !in_gap) {
      largest = std::max(largest, gap_size);

      gap_size = 0u;

      in_gap = true;
    }

    if (slot) {
      largest = std::max(largest, gap_size);

      gap_size = 0u;

      in_gap = false;
    }

    if (in_gap) {
      ++gap_size;
    }
  }

  return largest;
}

auto best_fit(const Spectrum &spectrum,
              const unsigned slots) -> std::optional<Slice> {
  assert(slots <= spectrum.size());

  if (slots > spectrum.largest_partition()) {
    return std::nullopt;
  }

  auto best_index{-1};

  auto min_block_size{std::numeric_limits<unsigned>::max()};

  auto current_block_size{0u};

  auto current_start_index{0u};

  for (auto index{0u}; index < spectrum.size(); ++index) {
    if (!spectrum.at(index)) {
      if (current_block_size == 0) {
        current_start_index = index;
      }

      ++current_block_size;

      continue;
    }

    if (current_block_size >= slots && current_block_size < min_block_size) {
      min_block_size = current_block_size;

      best_index = current_start_index;
    }

    current_block_size = 0;
  }

  if (current_block_size >= slots && current_block_size < min_block_size) {
    min_block_size = current_block_size;

    best_index = current_start_index;
  }

  if (best_index != -1) {
    return std::make_pair(static_cast<unsigned>(best_index),
                          static_cast<unsigned>(best_index) + slots - 1);
  }

  return std::nullopt;
}

auto first_fit(const Spectrum &spectrum,
               const unsigned slots) -> std::optional<Slice> {
  assert(slots <= spectrum.size());

  if (slots > spectrum.largest_partition()) {
    return std::nullopt;
  }

  const auto size = spectrum.size();

  for (auto start{0u}; start < size; ++start) {
    auto fit = true;

    for (auto end{0u}; end < slots && (start + end) < size; ++end) {
      if (spectrum.at(start + end)) {
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

    return std::make_pair(start, start + slots - 1);
  }

  return std::nullopt;
}

auto last_fit(const Spectrum &spectrum,
              const unsigned slots) -> std::optional<Slice> {
  assert(slots <= spectrum.size());

  if (slots > spectrum.largest_partition()) {
    return std::nullopt;
  }

  auto count{0u};

  for (auto index{spectrum.size() - 1}; 0 <= static_cast<int>(index); --index) {
    count = (!spectrum.at(index)) ? count + 1 : 0;

    if (count == slots) {
      return std::make_pair(index, index + slots - 1);
    }
  }

  return std::nullopt;
}

auto random_fit(const Spectrum &spectrum,
                const unsigned slots) -> std::optional<Slice> {
  assert(slots <= spectrum.size());

  if (slots > spectrum.largest_partition()) {
    return std::nullopt;
  }

  std::vector<unsigned> indexes{};

  for (auto start{0u}; start < spectrum.size(); ++start) {
    auto fit{true};

    for (auto end{0u}; end < slots; ++end) {
      if (spectrum.at(start + end)) {
        fit = false;

        break;
      }
    }

    if (!fit || start + slots - 1 >= spectrum.size()) {
      continue;
    }

    indexes.push_back(start);
  }

  if (indexes.empty()) {
    return std::nullopt;
  }

  std::random_device random_device;

  Uniform distribution{random_device(), 0, static_cast<double>(indexes.size())};

  const auto index{static_cast<unsigned>(distribution.next())};

  return std::make_pair(indexes[index], indexes[index] + slots - 1);
}

auto worst_fit(const Spectrum &spectrum,
               const unsigned slots) -> std::optional<Slice> {
  assert(slots <= spectrum.size());

  if (slots > spectrum.largest_partition()) {
    return std::nullopt;
  }

  auto max_block_size{0u};

  auto worst_index{-1};

  auto current_block_size{0u};

  auto current_start_index{0u};

  for (auto index{0u}; index < spectrum.size(); ++index) {
    if (!spectrum.at(index)) {
      if (current_block_size == 0) {
        current_start_index = index;
      }

      ++current_block_size;

      continue;
    }

    if (current_block_size >= slots && current_block_size > max_block_size) {
      max_block_size = current_block_size;

      worst_index = current_start_index;
    }

    current_block_size = 0;
  }

  if (current_block_size >= slots && current_block_size > max_block_size) {
    max_block_size = current_block_size;

    worst_index = current_start_index;
  }

  if (worst_index == -1) {
    return std::nullopt;
  }

  return std::make_pair(static_cast<unsigned>(worst_index),
                        static_cast<unsigned>(worst_index) + slots - 1);
}

auto fragmentation_coefficient(const Spectrum &spectrum) -> double {
  return 1.0 - (spectrum.largest_gap() / spectrum.size());
}

auto relative_fragmentation(const Spectrum &spectrum) -> double {
  return spectrum.gaps() / spectrum.size();
}

auto availability_ratio(const Spectrum &spectrum) -> double {
  return static_cast<double>(spectrum.available()) /
         static_cast<double>(spectrum.size());
}

auto utilization_ratio(const Spectrum &spectrum) -> double {
  return 1.0 - availability_ratio(spectrum);
}

auto shannon_entropy(const Spectrum &spectrum) -> double {
  const auto sar = availability_ratio(spectrum);

  const auto sur = utilization_ratio(spectrum);

  const auto coefficient_of = [](double x) { return x * std::log2(x); };

  if (sar == 0.0 || sur == 0.0) {
    return 0.0;
  }

  if (sar == 0.0 && sur != 0.0) {
    return coefficient_of(sur);
  }

  if (sar != 0.0 && sur == 0.0) {
    return coefficient_of(sar);
  }

  return -(coefficient_of(sur) + coefficient_of(sar));
}
