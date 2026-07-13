#include <cstdint>

#include <gtest/gtest.h>

#include <core/flexgrid.h>

TEST(Flexgrid, Allocation) {
  const uint16_t fsusPerCore = 5;

  const uint8_t coresPerFiber = 1;

  const uint16_t links = 2;

  core::Flexgrid flexgrid(fsusPerCore, coresPerFiber, links);

  const core::Flexgrid::Unit unit = {
    .fsu = 0,
    .link = 0,
    .core = 0,
  };

  ASSERT_EQ(flexgrid.IsAllocated(unit), false);

  flexgrid.Allocate(unit);

  ASSERT_EQ(flexgrid.IsAllocated(unit), true);

  flexgrid.Deallocate(unit);

  ASSERT_EQ(flexgrid.IsAllocated(unit), false);

}
