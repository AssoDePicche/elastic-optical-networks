#include "spectrum.h"

#include <prng/prng.h>

#include <algorithm>
#include <limits>
#include <numeric>
#include <random>
#include <ranges>

namespace core {
uint64_t size(const Slice& slice) { return slice.second - slice.first + 1; }

Spectrum::Spectrum(const uint64_t FSUsPerLink)
    : resources(std::vector(FSUsPerLink, FSU(false, 0u))) {
  slices.push_back({0, FSUsPerLink - 1});
}

void Spectrum::allocate(const Slice& slice) {
  const auto& [start, end] = slice;

  for (const auto index : std::ranges::views::iota(start, end + 1u)) {
    auto& [allocated, occupancy] = resources[index];

    allocated = true;

    ++occupancy;
  }

  auto iterator =
      std::ranges::find_if(slices, [&](const Slice& availableSlice) {
        const auto [i, j] = availableSlice;

        return i <= start && end <= j;
      });

  if (iterator == slices.end()) {
    return;
  }

  const auto& [iteratorStart, iteratorEnd] = *iterator;

  if (start == iteratorStart && end == iteratorEnd) {
    slices.erase(iterator);

    return;
  }

  if (start == iteratorStart) {
    *iterator = Slice(end + 1, iteratorEnd);

    return;
  }

  if (end == iteratorEnd) {
    *iterator = Slice(iteratorStart, start - 1);

    return;
  }

  const Slice before(iteratorStart, start - 1);

  const Slice after(end + 1, iteratorEnd);

  *iterator = before;

  slices.insert(iterator + 1, after);
}

void Spectrum::deallocate(const Slice& slice) {
  const auto& [start, end] = slice;

  for (const auto index : std::ranges::views::iota(start, end + 1u)) {
    auto& [allocated, occupancy] = resources[index];

    allocated = false;
  }

  auto iterator = std::lower_bound(
      slices.begin(), slices.end(), slice,
      [](const Slice& a, const Slice& b) { return a.second < b.first; });

  bool merged_previous = false;

  if (iterator != slices.begin()) {
    auto& [iteratorStart, iteratorEnd] = *(iterator - 1);

    if (iteratorEnd + 1 == start) {
      iteratorEnd = end;

      merged_previous = true;

      iterator = iterator - 1;
    }
  }

  auto& [iteratorStart, iteratorEnd] = *iterator;

  if (iterator != slices.end() && end + 1 == iteratorStart) {
    if (merged_previous) {
      iteratorStart = (iterator - 1)->first;

      iterator = slices.erase(iterator - 1);

      return;
    }

    iteratorStart = start;
  }

  if (!merged_previous) {
    slices.insert(iterator, slice);
  }
}

uint64_t Spectrum::size(void) const noexcept { return resources.size(); }

uint64_t Spectrum::available(void) const noexcept {
  return std::accumulate(slices.begin(), slices.end(), 0,
                         [](const int sum, const Slice& slice) {
                           const auto [start, end] = slice;

                           return sum + end - start + 1;
                         });
}

bool Spectrum::available_at(const Slice& slice) const noexcept {
  const auto& [start, end] = slice;

  const auto iterator = std::find_if(
      slices.begin(), slices.end(), [&](const Slice& availableSlice) {
        const auto [availableStart, availableEnd] = availableSlice;

        return start + end <= availableStart + availableEnd;
      });

  return iterator != slices.end();
}

std::vector<Slice> Spectrum::available_slices(void) const noexcept {
  return slices;
}

std::string Spectrum::Serialize(void) const noexcept {
  std::string buffer;

  std::for_each(resources.begin(), resources.end(),
                [&buffer](const FSU& resource) {
                  const auto& [allocated, occupancy] = resource;

                  buffer.append(allocated ? "#" : ".");
                });

  return buffer;
}

FSU Spectrum::at(const uint64_t index) const { return resources.at(index); }

std::optional<Slice> BestFit(const Spectrum& spectrum, const uint64_t FSUs) {
  const auto fit = [&](const uint64_t size) { return FSUs <= size; };

  const auto slices = spectrum.available_slices();

  auto buffer = slices | std::views::transform(size) | std::views::filter(fit);

  const auto iterator = std::min_element(buffer.begin(), buffer.end());

  if (iterator == buffer.end()) {
    return std::nullopt;
  }

  const auto min = *iterator;

  const auto minSize = [&](const Slice& slice) { return size(slice) == min; };

  const auto& [start, _] = *std::ranges::find_if(slices, minSize);

  return Slice(start, start + FSUs - 1);
}

std::optional<Slice> FirstFit(const Spectrum& spectrum, const uint64_t FSUs) {
  const auto slices = spectrum.available_slices();

  const auto iterator =
      std::find_if(slices.begin(), slices.end(), [=](const Slice& slice) {
        const auto [start, end] = slice;

        return start + FSUs - 1 <= end;
      });

  if (iterator == slices.end()) {
    return std::nullopt;
  }

  const auto& [start, _] = *iterator;

  return Slice(start, start + FSUs - 1);
}

std::optional<Slice> LastFit(const Spectrum& spectrum, const uint64_t FSUs) {
  const auto slices = spectrum.available_slices();

  const auto iterator =
      std::find_if(slices.rbegin(), slices.rend(), [=](const Slice& slice) {
        const auto [start, end] = slice;

        return start + FSUs - 1 <= end;
      });

  if (iterator == slices.rend()) {
    return std::nullopt;
  }

  const auto& [start, _] = *iterator;

  return Slice(start, start + FSUs - 1);
}

std::optional<Slice> RandomFit(const Spectrum& spectrum, const uint64_t FSUs) {
  const auto predicate = [&](const Slice& slice) {
    return FSUs <= size(slice);
  };

  auto filter = spectrum.available_slices() | std::views::filter(predicate);

  if (filter.empty()) {
    return std::nullopt;
  }

  std::vector<Slice> buffer(filter.begin(), filter.end());

  auto prng = prng::PseudoRandomNumberGenerator::Instance();

  prng->SetUniformVariable("random_fit", 0, buffer.size());

  const auto index = static_cast<uint64_t>(prng->Next("random_fit"));

  return buffer[index];
}

std::optional<Slice> WorstFit(const Spectrum& spectrum, const uint64_t FSUs) {
  const auto fit = [&](const uint64_t size) { return FSUs <= size; };

  const auto slices = spectrum.available_slices();

  auto buffer = slices | std::views::transform(size) | std::views::filter(fit);

  const auto iterator = std::max_element(buffer.begin(), buffer.end());

  if (iterator == buffer.end()) {
    return std::nullopt;
  }

  const auto max = *iterator;

  const auto maxSize = [&](const Slice& slice) { return size(slice) == max; };

  const auto& [start, _] = *std::ranges::find_if(slices, maxSize);

  return Slice(start, start + FSUs - 1);
}

double AbsoluteFragmentation::operator()(const Spectrum& spectrum) const {
  if (!spectrum.available()) {
    return .0f;
  }

  const auto buffer = spectrum.available_slices() | std::views::transform(size);

  const auto max = *std::max_element(buffer.begin(), buffer.end());

  return 1.f - max / static_cast<double>(spectrum.available());
}

double ExternalFragmentation::operator()(const Spectrum& spectrum) const {
  if (!spectrum.available()) {
    return .0f;
  }

  const auto buffer = spectrum.available_slices() | std::views::transform(size);

  const auto max = *std::max_element(buffer.begin(), buffer.end());

  return 1.f - max / static_cast<double>(spectrum.size());
}

EntropyBasedFragmentation::EntropyBasedFragmentation(const uint64_t minFSUs)
    : minFSUs{minFSUs} {}

double EntropyBasedFragmentation::operator()(const Spectrum& spectrum) const {
  if (!spectrum.available()) {
    return std::numeric_limits<double>::max();
  }

  const auto fit = [&](const uint64_t size) { return minFSUs <= size; };

  const auto FSUs = static_cast<double>(spectrum.size());

  const auto ratio = [&](const uint64_t& size) {
    return static_cast<double>(size) / FSUs;
  };

  const auto shannon = [&](const double& ratio) {
    return ratio * std::log(ratio);
  };

  auto buffer = spectrum.available_slices() | std::views::transform(size) |
                std::views::filter(fit) | std::views::transform(ratio) |
                std::views::transform(shannon);

  return -std::accumulate(buffer.begin(), buffer.end(), .0f);
}
}  // namespace core
