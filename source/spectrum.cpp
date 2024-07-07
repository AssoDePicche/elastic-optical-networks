#include "spectrum.h"

#include <algorithm>
#include <math.h>
#include <stdexcept>
#include <string>

Spectrum::Spectrum(const std::size_t size) : slots(std::vector(size, false)) {}

auto Spectrum::allocate(const std::size_t start, const std::size_t n) -> void {
  if (start + n > size() || n > available()) {
    throw std::out_of_range("Not enough slots");
  }

  for (auto index{start}; index < start + n; ++index) {
    if (slots[index]) {
      throw std::runtime_error("Slot " + std::to_string(index) +
                               " is already occupied");
    }
  }

  std::fill(slots.begin() + start, slots.begin() + start + n, true);
}

auto Spectrum::deallocate(const std::size_t start, const std::size_t n)
    -> void {
  if (start + n > size()) {
    throw std::out_of_range("Starting deallocation from invalid slot");
  }

  for (auto index{start}; index < start + n; ++index) {
    if (!slots[index]) {
      throw std::runtime_error("Slot " + std::to_string(index) +
                               " is already free");
    }
  }

  std::fill(slots.begin() + start, slots.begin() + start + n, false);
}

auto Spectrum::size(void) const noexcept -> std::size_t { return slots.size(); }

auto Spectrum::available(void) const noexcept -> std::size_t {
  return std::count(slots.begin(), slots.end(), false);
}

auto Spectrum::fragmentation(void) const noexcept -> double {
  auto free_blocks{0};

  auto block_size{0};

  auto in_free_block{false};

  std::vector<std::size_t> block_sizes{};

  for (const auto &slot : slots) {
    if (slot && in_free_block) {
      in_free_block = false;

      ++free_blocks;

      block_sizes.push_back(block_size);

      continue;
    }

    if (!slot) {
      if (!in_free_block) {
        in_free_block = true;

        block_size = 1;
      } else {
        ++block_size;
      }
    }
  }

  if (in_free_block) {
    ++free_blocks;

    block_sizes.push_back(block_size);
  }

  auto mean = static_cast<double>(available()) / free_blocks;

  double variance = 0.0;

  for (const auto &size : block_sizes) {
    variance += std::pow((size - mean), 2.0);
  }

  variance /= free_blocks;

  return free_blocks * std::sqrt(variance);
}

auto Spectrum::to_string(void) const noexcept -> std::string {
  std::string string;

  std::for_each(slots.begin(), slots.end(), [&string](const bool slot) {
    string.append(slot ? "#" : ".");
  });

  return string;
}
