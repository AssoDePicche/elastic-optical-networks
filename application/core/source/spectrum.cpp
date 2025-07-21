#include "spectrum.h"

#include <algorithm>
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

  for (const auto index : std::ranges::views::iota(start, end + 1)) {
    auto &[allocated, occupancy] = resources[index];

    allocated = true;

    ++occupancy;
  }

  auto iterator =
      std::ranges::find_if(availableSlices, [&](const Slice &availableSlice) {
        const auto [i, j] = availableSlice;

        return i <= start && end <= j;
      });

  if (iterator == availableSlices.end()) {
    return;
  }

  const auto &[iteratorStart, iteratorEnd] = *iterator;

  if (start == iteratorStart && end == iteratorEnd) {
    availableSlices.erase(iterator);

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

  availableSlices.insert(iterator + 1, after);
}

void Spectrum::deallocate(const Slice &slice) {
  const auto &[start, end] = slice;

  for (const auto index : std::ranges::views::iota(start, end + 1)) {
    auto &[allocated, occupancy] = resources[index];

    allocated = false;
  }

  auto iterator = std::lower_bound(
      availableSlices.begin(), availableSlices.end(), slice,
      [](const Slice &a, const Slice &b) { return a.second < b.first; });

  bool merged_previous = false;

  if (iterator != availableSlices.begin()) {
    auto &[iteratorStart, iteratorEnd] = *(iterator - 1);

    if (iteratorEnd + 1 == start) {
      iteratorEnd = end;

      merged_previous = true;

      iterator = iterator - 1;
    }
  }

  auto &[iteratorStart, iteratorEnd] = *iterator;

  if (iterator != availableSlices.end() && end + 1 == iteratorStart) {
    if (merged_previous) {
      iteratorStart = (iterator - 1)->first;

      iterator = availableSlices.erase(iterator - 1);

      return;
    }

    iteratorStart = start;
  }

  if (!merged_previous) {
    availableSlices.insert(iterator, slice);
  }
}

unsigned Spectrum::size(void) const noexcept { return resources.size(); }

unsigned Spectrum::available(void) const noexcept {
  return std::accumulate(
    availableSlices.begin(),
    availableSlices.end(),
    0,
    [](const int sum, const Slice &slice) {
      const auto [start, end] = slice;

      return sum + end - start + 1;
  });
}

bool Spectrum::available_at(const Slice &slice) const noexcept {
  const auto &[start, end] = slice;

  const auto iterator =
      std::find_if(availableSlices.begin(), availableSlices.end(),
                   [&](const Slice &availableSlice) {
                     const auto [availableStart, availableEnd] = availableSlice;

                     return start + end <= availableStart + availableEnd;
                   });

  return iterator != availableSlices.end();
}

std::vector<Slice> Spectrum::available_slices(void) const noexcept {
  return availableSlices;
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
  return resources.at(index);
}

std::optional<Slice> BestFit(const Spectrum &spectrum, const unsigned FSUs) {
  const auto size = [](const Slice& slice) { return slice.second - slice.first + 1; };

  const auto fit = [&](const unsigned size) { return FSUs <= size; };

  const auto slices = spectrum.available_slices();

  auto buffer = slices |
    std::views::transform(size) |
    std::views::filter(fit);

  const auto iterator = std::min_element(buffer.begin(), buffer.end());

  if (iterator == buffer.end()) {
    return std::nullopt;
  }

  const auto min = *iterator;

  const auto minSize = [&](const Slice& slice) {
    return slice.second - slice.first + 1 == min;
  };

  const auto &[start, _] = *std::ranges::find_if(slices, minSize);

  return Slice(start, start + FSUs - 1);
}

std::optional<Slice> FirstFit(const Spectrum &spectrum, const unsigned FSUs) {
  const auto availableSlices = spectrum.available_slices();

  const auto iterator = std::find_if(
    availableSlices.begin(),
    availableSlices.end(),
    [=](const Slice &slice) {
      const auto [start, end] = slice;

      return start + FSUs - 1 <= end;
  });

  if (iterator == availableSlices.end()) {
    return std::nullopt;
  }

  const auto& [start, _] = *iterator;

  return Slice(start, start + FSUs - 1);
}

std::optional<Slice> LastFit(const Spectrum &spectrum, const unsigned FSUs) {
  const auto availableSlices = spectrum.available_slices();

  const auto iterator = std::find_if(
    availableSlices.rbegin(),
    availableSlices.rend(),
    [=](const Slice &slice) {
      const auto [start, end] = slice;

      return start + FSUs - 1 <= end;
  });

  if (iterator == availableSlices.rend()) {
    return std::nullopt;
  }

  const auto& [start, _] = *iterator;

  return Slice(start, start + FSUs - 1);
}

std::optional<Slice> RandomFit(const Spectrum &spectrum, const unsigned FSUs) {
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
  const auto size = [](const Slice& slice) { return slice.second - slice.first + 1; };

  const auto fit = [&](const unsigned size) { return FSUs <= size; };

  const auto slices = spectrum.available_slices();

  auto buffer = slices |
    std::views::transform(size) |
    std::views::filter(fit);

  const auto iterator = std::max_element(buffer.begin(), buffer.end());

  if (iterator == buffer.end()) {
    return std::nullopt;
  }

  const auto max = *iterator;

  const auto maxSize = [&](const Slice& slice) {
    return slice.second - slice.first + 1 == max;
  };

  const auto &[start, _] = *std::ranges::find_if(slices, maxSize);

  return Slice(start, start + FSUs - 1);
}

double AbsoluteFragmentation::operator()(const Spectrum &spectrum) const {
  if (!spectrum.available()) {
    return .0f;
  }

  const auto buffer = spectrum.available_slices() |
    std::views::transform([](const Slice& slice) {
      return slice.second - slice.first + 1;
    }
  );

  const auto max = *std::max_element(buffer.begin(), buffer.end());

  return 1.f - max / static_cast<double>(spectrum.available());
}

double ExternalFragmentation::operator()(const Spectrum &spectrum) const {
  if (!spectrum.available()) {
    return 0;
  }

  const auto buffer = spectrum.available_slices() |
    std::views::transform([](const Slice& slice) {
      return slice.second - slice.first + 1;
    }
  );

  const auto max = *std::max_element(buffer.begin(), buffer.end());

  return 1.f - max / static_cast<double>(spectrum.size());
}

EntropyBasedFragmentation::EntropyBasedFragmentation(const unsigned minFSUs)
    : minFSUs{minFSUs} {}

double EntropyBasedFragmentation::operator()(const Spectrum &spectrum) const {
  if (!spectrum.available()) {
    return 0;
  }

  const auto size = [](const Slice &slice) { return slice.second - slice.first + 1; };

  const auto fit = [&](const unsigned size) { return minFSUs <= size; };

  const auto freeFSUs = static_cast<double>(spectrum.size());

  const auto ratio = [&](const unsigned &size) { return static_cast<double>(size) / freeFSUs; };

  const auto shannon = [&](const double &ratio) { return ratio * std::log(ratio); };

  auto buffer = spectrum.available_slices() |
    std::views::transform(size) |
    std::views::filter(fit) |
    std::views::transform(ratio) |
    std::views::transform(shannon);

  return -std::accumulate(buffer.begin(), buffer.end(), .0f);
}
