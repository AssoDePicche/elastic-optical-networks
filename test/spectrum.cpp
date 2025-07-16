#include "spectrum.h"

#include <gtest/gtest.h>

#include <vector>

TEST(Spectrum, FirstFit) {
  Spectrum spectrum(10);

  std::vector<Slice> slices = {
      {0, 3},
      {5, 7},
  };

  for (const auto& slice : slices) {
    spectrum.allocate(slice);
  }

  const Slice expected = {8, 9};

  const auto maybe = FirstFit(spectrum, 2);

  ASSERT_TRUE(maybe.has_value());

  ASSERT_EQ(maybe.value(), expected);
}
