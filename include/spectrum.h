#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

class Spectrum final {
public:
  Spectrum(const std::size_t);

  auto allocate(const std::size_t, const std::size_t) -> void;

  auto deallocate(const std::size_t, const std::size_t) -> void;

  [[nodiscard]] auto size(void) const noexcept -> std::size_t;

  [[nodiscard]] auto available(void) const noexcept -> std::size_t;

  [[nodiscard]] auto available_at(const std::size_t,
                                  const std::size_t) const noexcept -> bool;

  [[nodiscard]] auto available_partitions(void) const noexcept
      -> std::vector<std::size_t>;

  [[nodiscard]] auto available_partitions(const std::size_t) const noexcept
      -> std::vector<std::size_t>;

  [[nodiscard]] auto largest_partition(void) const noexcept -> std::size_t;

  [[nodiscard]] auto smallest_partition(void) const noexcept -> std::size_t;

  [[nodiscard]] auto fragmentation(void) const noexcept -> double;

  [[nodiscard]] auto fragmentation(const std::size_t) const noexcept -> double;

  [[nodiscard]] auto to_string(void) const noexcept -> std::string;

  [[nodiscard]] friend auto first_fit(const Spectrum &, const std::size_t)
      -> std::optional<std::size_t>;

private:
  std::vector<bool> slots;
};

using AllocationStrategy = std::function<std::optional<std::size_t>(
    const Spectrum &, const std::size_t)>;
