#include "arena.h"

#include <cassert>
#include <map>

namespace core {
struct Arena::Implementation {
  std::map<uint64_t, uint64_t> frequencyBuffer;
  std::unique_ptr<bool[]> buffer;
  uint64_t xSize;
  uint64_t ySize;
  uint64_t zSize;

  Implementation(uint64_t xSize, uint64_t ySize, uint64_t zSize)
      : xSize{xSize}, ySize{ySize}, zSize{zSize} {
    buffer = std::make_unique<bool[]>(xSize * ySize * zSize);
  }

  void Allocate(uint64_t x, uint64_t y, uint64_t z) {
    const uint64_t index = Index3D(x, y, z);

    assert(!buffer[index]);

    buffer[index] = true;

    ++frequencyBuffer[index];
  }

  void Deallocate(uint64_t x, uint64_t y, uint64_t z) {
    const uint64_t index = Index3D(x, y, z);

    assert(buffer[index]);

    buffer[index] = false;
  }

  bool IsAllocated(uint64_t x, uint64_t y, uint64_t z) const {
    const uint64_t index = Index3D(x, y, z);

    return buffer[index];
  }

  uint64_t GetTimesAllocated(uint64_t x, uint64_t y, uint64_t z) const {
    const uint64_t index = Index3D(x, y, z);

    return frequencyBuffer.at(index);
  }

  uint64_t Index3D(uint64_t x, uint64_t y, uint64_t z) const {
    return x + y * xSize + z * xSize * ySize;
  }
};

Arena::Arena(const uint64_t xSize, const uint64_t ySize, const uint64_t zSize) {
  pImpl = std::make_unique<Implementation>(xSize, ySize, zSize);
}

void Arena::Allocate(uint64_t x, uint64_t y, uint64_t z) {
  pImpl->Allocate(x, y, z);
}

void Arena::Deallocate(uint64_t x, uint64_t y, uint64_t z) {
  pImpl->Deallocate(x, y, z);
}

bool Arena::IsAllocated(uint64_t x, uint64_t y, uint64_t z) const {
  return pImpl->IsAllocated(x, y, z);
}

uint64_t Arena::GetTimesAllocated(uint64_t x, uint64_t y, uint64_t z) const {
  return pImpl->GetTimesAllocated(x, y, z);
}
};  // namespace core
