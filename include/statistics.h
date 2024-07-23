#pragma once

#include <functional>
#include <vector>

using Dataset = std::vector<double>;

using LossFunction = std::function<double(const Dataset &, const Dataset &)>;

[[nodiscard]] auto mean(const Dataset &) -> double;

[[nodiscard]] auto mean_absolute_error(const Dataset &, const Dataset &)
    -> double;

[[nodiscard]] auto mean_squared_error(const Dataset &, const Dataset &)
    -> double;

[[nodiscard]] auto root_mean_square_error(const Dataset &, const Dataset &)
    -> double;

#define MEAN(X) mean(X)

#define MAE(X, Y) mean_absolute_error(X, Y)

#define MSE(X, Y) mean_squared_error(X, Y)

#define RMSE(X, Y) root_mean_square_error(X, Y)
