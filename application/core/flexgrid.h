#pragma once

#include <cstdint>
#include <memory>

namespace core {
class Flexgrid final {
 public:
  struct Unit final {
    uint16_t fsu;
    uint16_t link;
    uint8_t core;
  };

  Flexgrid(const uint16_t, const uint8_t, const uint16_t);

  ~Flexgrid();

  Flexgrid(const Flexgrid&) = delete;

  Flexgrid& operator=(const Flexgrid&) = delete;

  Flexgrid(Flexgrid&&) noexcept;

  Flexgrid& operator=(Flexgrid&&) noexcept;

  void Allocate(const Unit);

  void Deallocate(const Unit);

  [[nodiscard]] bool IsAllocated(const Unit) const;

  [[nodiscard]] uint16_t GetTimesAllocated(const Unit) const;

 private:
  struct Implementation;
  std::unique_ptr<Implementation> pImpl;
};
};  // namespace core
