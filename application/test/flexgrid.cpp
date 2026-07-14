#include <core/flexgrid.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

TEST(Flexgrid, Allocation) {
  const uint16_t fsusPerCore = 5;

  const uint8_t coresPerFiber = 1;

  const uint16_t links = 2;

  core::Flexgrid flexgrid(fsusPerCore, coresPerFiber, links);

  std::vector<core::Flexgrid::Unit> units = {
      {
          .fsu = 0,
          .link = 0,
          .core = 0,

      },
      {
          .fsu = 1,
          .link = 0,
          .core = 0,

      },
  };

  for (const auto& unit : units) {
    ASSERT_FALSE(flexgrid.IsAllocated(unit));

    flexgrid.Allocate(unit);

    ASSERT_TRUE(flexgrid.IsAllocated(unit));

    flexgrid.Deallocate(unit);

    ASSERT_FALSE(flexgrid.IsAllocated(unit));
  }
}
