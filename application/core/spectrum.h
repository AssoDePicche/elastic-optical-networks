#pragma once

#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace core {
struct FSU final {
  bool allocated;
  uint64_t occupancy;

  static constexpr auto max = std::numeric_limits<uint64_t>::max();
};

using Slice = std::pair<uint64_t, uint64_t>;

[[nodiscard]] uint64_t size(const Slice&);

class Spectrum final {
 public:
  Spectrum(void) = default;

  Spectrum(const uint64_t);

  void allocate(const Slice&);

  void deallocate(const Slice&);

  [[nodiscard]] uint64_t size(void) const noexcept;

  [[nodiscard]] uint64_t available(void) const noexcept;

  [[nodiscard]] bool available_at(const Slice&) const noexcept;

  [[nodiscard]] std::vector<Slice> available_slices(void) const noexcept;

  [[nodiscard]] std::string Serialize(void) const noexcept;

  [[nodiscard]] FSU at(const uint64_t) const;

 private:
  std::vector<FSU> resources;
  std::vector<Slice> slices;
};

using Carriers = std::unordered_map<uint64_t, Spectrum>;

[[nodiscard]] std::optional<Slice> BestFit(const Spectrum&, const uint64_t);

[[nodiscard]] std::optional<Slice> FirstFit(const Spectrum&, const uint64_t);

[[nodiscard]] std::optional<Slice> LastFit(const Spectrum&, const uint64_t);

[[nodiscard]] std::optional<Slice> RandomFit(const Spectrum&, const uint64_t);

[[nodiscard]] std::optional<Slice> WorstFit(const Spectrum&, const uint64_t);

using SpectrumAllocator =
    std::function<std::optional<Slice>(const Spectrum&, const uint64_t)>;

struct Fragmentation {
  virtual ~Fragmentation() = default;

  [[nodiscard]] virtual double operator()(const Spectrum&) const = 0;
};

using FragmentationStrategy = std::shared_ptr<Fragmentation>;

struct AbsoluteFragmentation : public Fragmentation {
  [[nodiscard]] double operator()(const Spectrum&) const override;
};

struct ExternalFragmentation : public Fragmentation {
  [[nodiscard]] double operator()(const Spectrum&) const override;
};

struct EntropyBasedFragmentation : public Fragmentation {
  uint64_t minFSUs;

  EntropyBasedFragmentation(const uint64_t);

  [[nodiscard]] double operator()(const Spectrum&) const override;
};
}  // namespace core
