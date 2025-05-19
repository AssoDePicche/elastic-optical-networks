#include "spectrum.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>
#include <random>

#include "distribution.h"

Spectrum::Spectrum(const unsigned FSUsPerLink)
    : resources(std::vector(FSUsPerLink, FSU(false, 0u))) {}

auto Spectrum::allocate(const Slice &slice) -> void {
  const auto &[start, end] = slice;

  assert(end < size());

  assert(available_at(slice));

  auto i = resources.begin() + start;

  const auto j = resources.begin() + end + 1;

  while (i != j) {
    auto &[allocated, occupancy] = *i;

    allocated = true;

    ++occupancy;

    ++i;
  }
}

auto Spectrum::deallocate(const Slice &slice) -> void {
  const auto &[start, end] = slice;

  assert(end < size());

  assert(!available_at(slice));

  auto i = resources.begin() + start;

  const auto j = resources.begin() + end + 1;

  while (i != j) {
    auto &[allocated, occupancy] = *i;

    allocated = false;

    ++i;
  }
}

auto Spectrum::size(void) const noexcept -> unsigned {
  return resources.size();
}

auto Spectrum::available(void) const noexcept -> unsigned {
  auto count = 0u;

  for (const auto &[allocated, occupancy] : resources) {
    if (!allocated) {
      ++count;
    }
  }

  return count;
}

auto Spectrum::available_at(const Slice &slice) const noexcept -> bool {
  const auto &[start, end] = slice;

  auto i = resources.begin() + start;

  const auto j = resources.begin() + end + 1;

  while (i != j) {
    const auto &[allocated, occupancy] = *i;

    if (allocated) {
      return false;
    }

    ++i;
  }

  return true;
}

auto Spectrum::available_partitions(void) const noexcept
    -> std::vector<unsigned> {
  auto length{0};

  auto in_free_block{false};

  std::vector<unsigned> partitions{};

  for (const auto &[allocated, occupancy] : resources) {
    if (allocated && in_free_block) {
      in_free_block = false;

      partitions.push_back(length);

      continue;
    }

    if (allocated) {
      continue;
    }

    if (in_free_block) {
      ++length;

      continue;
    }

    in_free_block = true;

    length = 1;
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

auto Spectrum::Serialize(void) const noexcept -> std::string {
  std::string buffer;

  std::for_each(resources.begin(), resources.end(),
                [&buffer](const FSU &resource) {
                  const auto &[allocated, occupancy] = resource;

                  buffer.append(allocated ? "#" : ".");
                });

  return buffer;
}

auto Spectrum::at(const unsigned index) const -> FSU {
  assert(index < size());

  return resources.at(index);
}

auto Spectrum::gaps(void) const -> unsigned {
  auto gaps = 0u;

  auto in_gap = false;

  for (const auto &[allocated, occupancy] : resources) {
    if (!allocated && in_gap) {
      continue;
    }

    if (!allocated && !in_gap) {
      in_gap = true;
    }

    if (allocated) {
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

  for (const auto &[allocated, occupancy] : resources) {
    if (!allocated && in_gap) {
      ++gap_size;

      continue;
    }

    if (!allocated && !in_gap) {
      largest = std::max(largest, gap_size);

      gap_size = 0u;

      in_gap = true;
    }

    if (allocated) {
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

std::optional<Slice> BestFit(const Spectrum &spectrum, const unsigned FSUs) {
  assert(FSUs <= spectrum.size());

  if (FSUs > spectrum.largest_partition()) {
    return std::nullopt;
  }

  auto best_index{-1};

  auto min_block_size{std::numeric_limits<unsigned>::max()};

  auto current_block_size{0u};

  auto current_start_index{0u};

  for (auto index{0u}; index < spectrum.size(); ++index) {
    const auto &[allocated, occupancy] = spectrum.at(index);

    if (!allocated && current_block_size == 0u) {
      current_start_index = index;
    }

    if (!allocated) {
      ++current_block_size;

      continue;
    }

    if (current_block_size >= FSUs && current_block_size < min_block_size) {
      min_block_size = current_block_size;

      best_index = current_start_index;
    }

    current_block_size = 0u;
  }

  if (current_block_size >= FSUs && current_block_size < min_block_size) {
    min_block_size = current_block_size;

    best_index = current_start_index;
  }

  if (best_index == -1) {
    return std::nullopt;
  }

  return Slice(static_cast<unsigned>(best_index),
               static_cast<unsigned>(best_index) + FSUs - 1);
}

std::optional<Slice> FirstFit(const Spectrum &spectrum, const unsigned FSUs) {
  assert(FSUs <= spectrum.size());

  if (FSUs > spectrum.largest_partition()) {
    return std::nullopt;
  }

  const auto size = spectrum.size();

  for (auto start{0u}; start < size; ++start) {
    auto fit = true;

    for (auto end{0u}; end < FSUs && (start + end) < size; ++end) {
      const auto &[allocated, occupancy] = spectrum.at(start + end);

      if (allocated) {
        fit = false;

        break;
      }
    }

    if (!fit) {
      continue;
    }

    if (start + FSUs - 1 >= spectrum.size()) {
      return std::nullopt;
    }

    return Slice(start, start + FSUs - 1);
  }

  return std::nullopt;
}

std::optional<Slice> LastFit(const Spectrum &spectrum, const unsigned FSUs) {
  assert(FSUs <= spectrum.size());

  if (FSUs > spectrum.largest_partition()) {
    return std::nullopt;
  }

  auto count{0u};

  for (auto index{static_cast<int>(spectrum.size()) - 1}; 0 <= index; --index) {
    const auto &[allocated, occupancy] = spectrum.at(index);

    if (!allocated) {
      ++count;
    }

    if (count == FSUs) {
      return Slice(index, index + FSUs - 1);
    }
  }

  return std::nullopt;
}

std::optional<Slice> RandomFit(const Spectrum &spectrum, const unsigned FSUs) {
  assert(FSUs <= spectrum.size());

  if (FSUs > spectrum.largest_partition()) {
    return std::nullopt;
  }

  std::vector<unsigned> indexes{};

  for (auto start{0u}; start < spectrum.size(); ++start) {
    auto fit{true};

    for (auto end{0u}; end < FSUs; ++end) {
      const auto &[allocated, occupancy] = spectrum.at(start + end);

      if (allocated) {
        fit = false;

        break;
      }
    }

    if (!fit || start + FSUs - 1 >= spectrum.size()) {
      continue;
    }

    indexes.push_back(start);
  }

  if (indexes.empty()) {
    return std::nullopt;
  }

  static std::random_device random_device;

  static Uniform distribution{random_device(), 0,
                              static_cast<double>(indexes.size())};

  const auto index{static_cast<unsigned>(distribution.next())};

  return Slice(indexes.at(index), indexes.at(index) + FSUs - 1);
}

std::optional<Slice> WorstFit(const Spectrum &spectrum, const unsigned FSUs) {
  assert(FSUs <= spectrum.size());

  if (FSUs > spectrum.largest_partition()) {
    return std::nullopt;
  }

  auto max_block_size{0u};

  auto worst_index{-1};

  auto current_block_size{0u};

  auto current_start_index{0u};

  for (auto index{0u}; index < spectrum.size(); ++index) {
    const auto &[allocated, occupancy] = spectrum.at(index);

    if (!allocated) {
      if (current_block_size == 0) {
        current_start_index = index;
      }

      ++current_block_size;

      continue;
    }

    if (current_block_size >= FSUs && current_block_size > max_block_size) {
      max_block_size = current_block_size;

      worst_index = current_start_index;
    }

    current_block_size = 0;
  }

  if (current_block_size >= FSUs && current_block_size > max_block_size) {
    max_block_size = current_block_size;

    worst_index = current_start_index;
  }

  if (worst_index == -1) {
    return std::nullopt;
  }

  const auto start = static_cast<unsigned>(worst_index);

  return Slice(start, start + FSUs - 1);
}

double ExternalFragmentation::operator()(const Spectrum &spectrum) const {
  return 1.0 - (spectrum.largest_gap() / spectrum.size());
}

EntropyBasedFragmentation::EntropyBasedFragmentation(const unsigned minFSUs)
    : minFSUs{minFSUs} {}

double EntropyBasedFragmentation::operator()(const Spectrum &spectrum) const {
  const auto partitions = spectrum.available_partitions(minFSUs).size();

  if (0u == partitions) {
    return 0.0;
  }

  const auto S = static_cast<double>(spectrum.size());

  return (partitions / S) * std::log(S / partitions);
}
