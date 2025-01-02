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

#define MEAN(X) mean(X)

#define MAE(X, Y) mean_absolute_error(X, Y)

#define MSE(X, Y) mean_squared_error(X, Y)

#define RMSE(X, Y) root_mean_square_error(X, Y)

#define STDDEV(X) standard_deviation(X)
