#pragma once

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using Slice = std::pair<unsigned, unsigned>;

class Spectrum final {
 public:
  Spectrum(void) = default;

  Spectrum(const unsigned);

  auto allocate(const Slice &) -> void;

  auto deallocate(const Slice &) -> void;

  [[nodiscard]] auto size(void) const noexcept -> unsigned;

  [[nodiscard]] auto available(void) const noexcept -> unsigned;

  [[nodiscard]] auto available_at(const Slice &) const noexcept -> bool;

  [[nodiscard]] auto available_partitions(void) const noexcept
      -> std::vector<unsigned>;

  [[nodiscard]] auto available_partitions(const unsigned) const noexcept
      -> std::vector<unsigned>;

  [[nodiscard]] auto largest_partition(void) const noexcept -> unsigned;

  [[nodiscard]] auto smallest_partition(void) const noexcept -> unsigned;

  [[nodiscard]] auto fragmentation(void) const noexcept -> double;

  [[nodiscard]] auto fragmentation(const unsigned) const noexcept -> double;

  [[nodiscard]] auto to_string(void) const noexcept -> std::string;

  [[nodiscard]] auto at(const unsigned) const -> std::pair<bool, unsigned>;

  [[nodiscard]] auto gaps(void) const -> unsigned;

  [[nodiscard]] auto largest_gap(void) const -> unsigned;

 private:
  std::vector<std::pair<bool, unsigned>> resources;
};

[[nodiscard]] auto best_fit(const Spectrum &,
                            const unsigned) -> std::optional<Slice>;

[[nodiscard]] auto first_fit(const Spectrum &,
                             const unsigned) -> std::optional<Slice>;

[[nodiscard]] auto last_fit(const Spectrum &,
                            const unsigned) -> std::optional<Slice>;

[[nodiscard]] auto random_fit(const Spectrum &,
                              const unsigned) -> std::optional<Slice>;

[[nodiscard]] auto worst_fit(const Spectrum &,
                             const unsigned) -> std::optional<Slice>;

using SpectrumAllocator =
    std::function<std::optional<Slice>(const Spectrum &, const unsigned)>;

[[nodiscard]] auto fragmentation_coefficient(const Spectrum &) -> double;

[[nodiscard]] auto relative_fragmentation(const Spectrum &) -> double;

[[nodiscard]] auto availability_ratio(const Spectrum &) -> double;

[[nodiscard]] auto utilization_ratio(const Spectrum &) -> double;

using FragmentationMetric = std::function<double(const Spectrum &)>;

[[nodiscard]] auto shannon_entropy(const Spectrum &) -> double;
