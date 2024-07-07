#pragma once

#include <ostream>
#include <vector>

class Spectrum final {
public:
  Spectrum(const std::size_t);

  auto allocate(const std::size_t, const std::size_t) -> void;

  auto deallocate(const std::size_t, const std::size_t) -> void;

  auto size(void) const noexcept -> std::size_t;

  auto available(void) const noexcept -> std::size_t;

  auto fragmentation(void) const noexcept -> double;

  [[nodiscard]] auto to_string(void) const noexcept -> std::string;

private:
  std::vector<bool> slots;
};
