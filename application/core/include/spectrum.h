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

  void allocate(const Slice &);

  void deallocate(const Slice &);

  [[nodiscard]] unsigned size(void) const noexcept;

  [[nodiscard]] unsigned available(void) const noexcept;

  [[nodiscard]] bool available_at(const Slice &) const noexcept;

  [[nodiscard]] std::vector<unsigned> available_partitions(void) const noexcept;

  [[nodiscard]] std::vector<unsigned> available_partitions(
      const unsigned) const noexcept;

  [[nodiscard]] unsigned largest_partition(void) const noexcept;

  [[nodiscard]] unsigned smallest_partition(void) const noexcept;

  [[nodiscard]] std::string Serialize(void) const noexcept;

  [[nodiscard]] FSU at(const unsigned) const;

  [[nodiscard]] unsigned gaps(void) const;

  [[nodiscard]] unsigned largest_gap(void) const;

 private:
  std::vector<FSU> resources;
  std::vector<Slice> availableSlices;
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

struct AbsoluteFragmentation : public Fragmentation {
  [[nodiscard]] double operator()(const Spectrum &) const override;
};

struct ExternalFragmentation : public Fragmentation {
  [[nodiscard]] double operator()(const Spectrum &) const override;
};

struct EntropyBasedFragmentation : public Fragmentation {
  unsigned minFSUs;

  EntropyBasedFragmentation(const unsigned);

  [[nodiscard]] double operator()(const Spectrum &) const override;
};
