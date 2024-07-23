#include "statistics.h"

#include <cassert>
#include <cmath>
#include <numeric>

auto mean(const Dataset &dataset) -> double {
  const auto sum{std::accumulate(dataset.begin(), dataset.end(), 0.0)};

  return (sum / dataset.size());
}

auto mean_absolute_error(const Dataset &actual, const Dataset &predicted)
    -> double {
  assert(actual.size() == predicted.size());

  Dataset absolute_errors;

  for (auto index{0u}; index < actual.size(); ++index) {
    const auto error = actual[index] - predicted[index];

    absolute_errors.push_back(std::abs(error));
  }

  return mean(absolute_errors);
}

auto mean_squared_error(const Dataset &actual, const Dataset &predicted)
    -> double {
  assert(actual.size() == predicted.size());

  Dataset squared_differences;

  for (auto index{0u}; index < actual.size(); ++index) {
    const auto difference = actual[index] - predicted[index];

    squared_differences.push_back(std::pow(difference, 2.0));
  }

  return mean(squared_differences);
}

auto root_mean_square_error(const Dataset &actual, const Dataset &predicted)
    -> double {
  assert(actual.size() == predicted.size());

  return std::sqrt(mean_squared_error(actual, predicted));
}
