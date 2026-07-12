#include "flexgrid.h"

#include <cassert>

namespace core {
struct FSU final {
  uint16_t timesAllocated{0};
  bool isAllocated{false};

  void Allocate() {
    assert(!isAllocated);

    isAllocated = true;

    ++timesAllocated;
  }

  void Deallocate() {
    assert(isAllocated);

    isAllocated = false;
  }
};

struct Flexgrid::Implementation final {
  Implementation(const uint16_t fsusPerCore, const uint8_t coresPerFiber,
                 const uint16_t links)
      : fsusPerCore{fsusPerCore}, coresPerFiber{coresPerFiber}, links{links} {
    const size_t totalSize =
        static_cast<size_t>(fsusPerCore * coresPerFiber * links);

    buffer = std::make_unique<FSU[]>(totalSize);
  }

  void Allocate(const Unit unit) { GetFSU(unit).Allocate(); }

  void Deallocate(const Unit unit) { GetFSU(unit).Deallocate(); }

  bool IsAllocated(const Unit unit) const { return GetFSU(unit).isAllocated; }

  uint16_t GetTimesAllocated(const Unit unit) const {
    return GetFSU(unit).timesAllocated;
  }

 private:
  std::unique_ptr<FSU[]> buffer;
  uint16_t fsusPerCore;
  uint8_t coresPerFiber;
  uint16_t links;

  size_t Index3D(const Unit unit) const {
    const auto& [fsu, link, core] = unit;

    assert(fsu < fsusPerCore);

    assert(core < coresPerFiber);

    assert(link < links);

    return static_cast<size_t>(fsu) + static_cast<size_t>(core) * fsusPerCore +
           static_cast<size_t>(link) * fsusPerCore * coresPerFiber;
  }

  FSU& GetFSU(const Unit unit) { return buffer[Index3D(unit)]; }

  const FSU& GetFSU(const Unit unit) const { return buffer[Index3D(unit)]; }
};

Flexgrid::Flexgrid(const uint16_t fsusPerCore, const uint8_t coresPerFiber,
                   const uint16_t links) {
  pImpl = std::make_unique<Implementation>(fsusPerCore, coresPerFiber, links);
}

Flexgrid::~Flexgrid() = default;

Flexgrid::Flexgrid(Flexgrid&&) noexcept = default;

Flexgrid& Flexgrid::operator=(Flexgrid&&) noexcept = default;

void Flexgrid::Allocate(const Unit unit) { pImpl->Allocate(unit); }

void Flexgrid::Deallocate(const Unit unit) { pImpl->Deallocate(unit); }

bool Flexgrid::IsAllocated(const Unit unit) const {
  return pImpl->IsAllocated(unit);
}

uint16_t Flexgrid::GetTimesAllocated(const Unit unit) const {
  return pImpl->GetTimesAllocated(unit);
}
};  // namespace core
