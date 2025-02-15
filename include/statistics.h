#pragma once

#include <vector>

[[nodiscard]] auto mean(const std::vector<double> &) -> double;

[[nodiscard]] auto mean_absolute_error(const std::vector<double> &,
                                       const std::vector<double> &) -> double;

[[nodiscard]] auto mean_squared_error(const std::vector<double> &,
                                      const std::vector<double> &) -> double;

[[nodiscard]] auto root_mean_square_error(
    const std::vector<double> &, const std::vector<double> &) -> double;

[[nodiscard]] auto standard_deviation(const std::vector<double> &) -> double;

[[nodiscard]] bool is_equal(const double, const double, const double) noexcept;

#define mae(X, Y) mean_absolute_error(X, Y)

#define mse(X, Y) mean_squared_error(X, Y)

#define rmse(X, Y) root_mean_square_error(X, Y)

#define stddev(X) standard_deviation(X)
