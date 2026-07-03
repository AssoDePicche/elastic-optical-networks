#pragma once

#include <cstdint>
#include <memory>

namespace core {
class Arena final {
 public:
  Arena(const uint64_t, const uint64_t, const uint64_t);

  ~Arena();

  Arena(const Arena&) = delete;

  Arena& operator=(const Arena&) = delete;

  Arena(Arena&&) noexcept;

  Arena& operator=(Arena&&) noexcept;

  void Allocate(uint64_t, uint64_t, uint64_t);

  void Deallocate(uint64_t, uint64_t, uint64_t);

  [[nodiscard]] bool IsAllocated(uint64_t, uint64_t, uint64_t) const;

  [[nodiscard]] uint64_t GetTimesAllocated(uint64_t, uint64_t, uint64_t) const;

 private:
  struct Implementation;
  std::unique_ptr<Implementation> pImpl;
};
};  // namespace core
