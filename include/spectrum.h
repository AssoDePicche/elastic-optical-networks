#pragma once

#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct FSU final {
  bool allocated;
  unsigned occupancy;

  static constexpr auto max = std::numeric_limits<unsigned>::max();
};

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

  [[nodiscard]] auto Serialize(void) const noexcept -> std::string;

  [[nodiscard]] auto at(const unsigned) const -> FSU;

  [[nodiscard]] auto gaps(void) const -> unsigned;

  [[nodiscard]] auto largest_gap(void) const -> unsigned;

 private:
  std::vector<FSU> resources;
};

[[nodiscard]] std::optional<Slice> BestFit(const Spectrum &, const unsigned);

[[nodiscard]] std::optional<Slice> FirstFit(const Spectrum &, const unsigned);

[[nodiscard]] std::optional<Slice> LastFit(const Spectrum &, const unsigned);

[[nodiscard]] std::optional<Slice> RandomFit(const Spectrum &, const unsigned);

[[nodiscard]] std::optional<Slice> WorstFit(const Spectrum &, const unsigned);

using SpectrumAllocator =
    std::function<std::optional<Slice>(const Spectrum &, const unsigned)>;

struct Fragmentation {
  virtual ~Fragmentation() = default;

  [[nodiscard]] virtual double operator()(const Spectrum &) const = 0;
};

using FragmentationStrategy = std::shared_ptr<Fragmentation>;

struct ExternalFragmentation : public Fragmentation {
  [[nodiscard]] double operator()(const Spectrum &) const override;
};

struct EntropyBasedFragmentation : public Fragmentation {
  unsigned minFSUs;

  EntropyBasedFragmentation(const unsigned);

  [[nodiscard]] double operator()(const Spectrum &) const override;
};
