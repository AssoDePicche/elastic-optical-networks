#pragma once

#include <cmath>
#include <functional>
#include <iterator>
#include <numeric>

using PairingFunction = std::function<unsigned(unsigned, unsigned)>;

[[nodiscard]] unsigned CantorPairingFunction(unsigned, unsigned);

template <class Iterator>
[[nodiscard]] double Mean(Iterator first, Iterator last) {
  return std::accumulate(first, last, 0.0) / std::distance(first, last);
}

template <class Iterator>
[[nodiscard]] double StandardDeviation(Iterator first, Iterator last) {
  const auto mean = Mean(first, last);

  const auto variance = std::accumulate(
      first, last, 0.0,
      [mean](double sum, double x) { return sum + (x - mean) * (x - mean); });

  return std::sqrt(variance);
}
