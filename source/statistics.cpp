#include "statistics.h"

#include <cassert>
#include <cmath>
#include <numeric>

auto mean(const std::vector<double> &dataset) -> double {
  if (dataset.size() == 0) {
    return 0.0;
  }

  return std::accumulate(dataset.begin(), dataset.end(), 0.0) / dataset.size();
}

auto mean_absolute_error(const std::vector<double> &actual,
                         const std::vector<double> &predicted) -> double {
  assert(actual.size() == predicted.size());

  std::vector<double> absolute_errors;

  for (auto index{0u}; index < actual.size(); ++index) {
    const auto error = actual[index] - predicted[index];

    absolute_errors.push_back(std::abs(error));
  }

  return mean(absolute_errors);
}

auto mean_squared_error(const std::vector<double> &actual,
                        const std::vector<double> &predicted) -> double {
  assert(actual.size() == predicted.size());

  std::vector<double> squared_differences;

  for (auto index{0u}; index < actual.size(); ++index) {
    const auto difference = actual[index] - predicted[index];

    squared_differences.push_back(difference * difference);
  }

  return mean(squared_differences);
}

auto root_mean_square_error(const std::vector<double> &actual,
                            const std::vector<double> &predicted) -> double {
  assert(actual.size() == predicted.size());

  return std::sqrt(mean_squared_error(actual, predicted));
}

auto standard_deviation(const std::vector<double> &dataset) -> double {
  if (dataset.size() == 0) {
    return 0.0;
  }

  const auto x_mean = mean(dataset);

  const auto variance =
      std::accumulate(dataset.begin(), dataset.end(), 0.0,
                      [x_mean](double sum, double x) {
                        return sum + (x - x_mean) * (x - x_mean);
                      }) /
      dataset.size();

  return std::sqrt(variance);
}

bool is_equal(const double x, const double y, const double tolerance) noexcept {
  return std::abs(x - y) < tolerance;
}
