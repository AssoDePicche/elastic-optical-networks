#include "spectrum.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>
#include <random>
#include <ranges>

#include "distribution.h"

Spectrum::Spectrum(const unsigned FSUsPerLink)
    : resources(std::vector(FSUsPerLink, FSU(false, 0u))) {
  availableSlices.push_back({0, FSUsPerLink - 1});
}

void Spectrum::allocate(const Slice &slice) {
  const auto &[start, end] = slice;

  assert(end < size());

  assert(available_at(slice));

  for (const auto index : std::ranges::views::iota(start, end + 1)) {
    auto &[allocated, occupancy] = resources[index];

    allocated = true;

    ++occupancy;
  }
}

void Spectrum::deallocate(const Slice &slice) {
  const auto &[start, end] = slice;

  assert(end < size());

  assert(!available_at(slice));

  for (const auto index : std::ranges::views::iota(start, end + 1)) {
    auto &[allocated, occupancy] = resources[index];

    allocated = false;

    ++occupancy;
  }
}

unsigned Spectrum::size(void) const noexcept { return resources.size(); }

unsigned Spectrum::available(void) const noexcept {
  auto count = 0u;

  for (const auto &[allocated, occupancy] : resources) {
    if (!allocated) {
      ++count;
    }
  }

  return count;
}

bool Spectrum::available_at(const Slice &slice) const noexcept {
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

std::vector<unsigned> Spectrum::available_partitions(void) const noexcept {
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

std::vector<unsigned> Spectrum::available_partitions(
    const unsigned size) const noexcept {
  const auto partitions = available_partitions();

  std::vector<unsigned> suitable;

  std::copy_if(partitions.begin(), partitions.end(),
               std::back_inserter(suitable),
               [&](const unsigned capacity) { return size <= capacity; });

  return suitable;
}

unsigned Spectrum::largest_partition(void) const noexcept {
  const auto partitions = available_partitions();

  const auto ptr = std::max_element(partitions.begin(), partitions.end());

  return (ptr == partitions.end()) ? 0 : *ptr;
}

unsigned Spectrum::smallest_partition(void) const noexcept {
  const auto partitions = available_partitions();

  const auto ptr = std::min_element(partitions.begin(), partitions.end());

  return (ptr == partitions.end()) ? 0 : *ptr;
}

std::string Spectrum::Serialize(void) const noexcept {
  std::string buffer;

  std::for_each(resources.begin(), resources.end(),
                [&buffer](const FSU &resource) {
                  const auto &[allocated, occupancy] = resource;

                  buffer.append(allocated ? "#" : ".");
                });

  return buffer;
}

FSU Spectrum::at(const unsigned index) const {
  assert(index < size());

  return resources.at(index);
}

unsigned Spectrum::gaps(void) const {
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

unsigned Spectrum::largest_gap(void) const {
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

  const auto size = spectrum.size();

  std::optional<unsigned> best_index;

  unsigned min_block_size = std::numeric_limits<unsigned>::max();

  unsigned current_start = 0;

  unsigned current_length = 0;

  for (const auto index : std::views::iota(0u, size)) {
    const auto &[allocated, _] = spectrum.at(index);

    if (!allocated) {
      if (current_length == 0) {
        current_start = index;
      }

      ++current_length;
    }

    if (allocated || index == size - 1) {
      if (FSUs <= current_length && current_length < min_block_size) {
        best_index = current_start;

        min_block_size = current_length;
      }

      current_length = 0;
    }
  }

  if (!best_index) {
    return std::nullopt;
  }

  return Slice(*best_index, *best_index + FSUs - 1);
}

std::optional<Slice> FirstFit(const Spectrum &spectrum, const unsigned FSUs) {
  assert(FSUs <= spectrum.size());

  if (FSUs > spectrum.largest_partition()) {
    return std::nullopt;
  }

  const auto candidates = std::views::iota(0u, spectrum.size() - FSUs + 1);

  const auto is_available = [](const auto &FSU) { return !FSU.allocated; };

  for (const auto index : candidates) {
    const auto range = std::views::iota(0u, FSUs) |
                       std::views::transform([&](const auto offset) {
                         return spectrum.at(index + offset);
                       });

    if (std::ranges::all_of(range, is_available)) {
      return Slice(index, index + FSUs - 1);
    }
  }

  return std::nullopt;
}

std::optional<Slice> LastFit(const Spectrum &spectrum, const unsigned FSUs) {
  assert(FSUs <= spectrum.size());

  if (FSUs > spectrum.largest_partition()) {
    return std::nullopt;
  }

  unsigned count = 0;

  const auto range =
      std::views::iota(0u, spectrum.size()) | std::views::reverse;

  for (const auto index : range) {
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

  std::optional<unsigned> worst_index;

  unsigned max_block_size = 0;

  unsigned current_start = 0;

  unsigned current_length = 0;

  for (const auto index : std::views::iota(0u, spectrum.size())) {
    const auto &[allocated, _] = spectrum.at(index);

    if (!allocated) {
      if (current_length == 0) {
        current_start = index;
      }

      ++current_length;
    }

    if (allocated || index == spectrum.size() - 1) {
      if (FSUs <= current_length && current_length > max_block_size) {
        max_block_size = current_length;

        worst_index = current_start;
      }

      current_length = 0;
    }
  }

  if (!worst_index) {
    return std::nullopt;
  }

  return Slice(*worst_index, *worst_index + FSUs - 1);
}

double AbsoluteFragmentation::operator()(const Spectrum &spectrum) const {
  const auto total = static_cast<double>(spectrum.available());

  if (total == 0.0) {
    return 0.0;
  }

  return (total - spectrum.largest_partition()) / total;
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
