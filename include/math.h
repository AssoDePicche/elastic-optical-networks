#pragma once

#include <cmath>
#include <functional>
#include <iterator>
#include <numeric>

using PairingFunction = std::function<unsigned(unsigned, unsigned)>;

[[nodiscard]] unsigned CantorPairingFunction(unsigned, unsigned);

struct Unit final {
  double mean;
  double stddev;
  double variance;

  Unit(double, double, double);

  template <class Iterator>
  [[nodiscard]] static Unit New(Iterator first, Iterator last) {
    const auto mean =
        std::accumulate(first, last, 0.0) / std::distance(first, last);

    const auto variance =
        std::accumulate(first, last, 0.0, [&mean](double sum, double x) {
          return sum + (x - mean) * (x - mean);
        });

    const auto stddev = std::sqrt(variance);

    return Unit(mean, stddev, variance);
  }
};
