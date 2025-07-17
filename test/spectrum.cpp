#include "spectrum.h"

#include <gtest/gtest.h>

#include <vector>

TEST(Spectrum, BestFit) {
  Spectrum spectrum(10);

  spectrum.allocate({0, 1});

  spectrum.allocate({5, 7});

  const Slice expected = {8, 9};

  const auto maybe = BestFit(spectrum, 2);

  ASSERT_TRUE(maybe.has_value());

  ASSERT_EQ(maybe.value(), expected);
}

TEST(Spectrum, FirstFit) {
  Spectrum spectrum(10);

  spectrum.allocate({0, 1});

  spectrum.allocate({5, 7});

  const Slice expected = {2, 3};

  const auto maybe = FirstFit(spectrum, 2);

  ASSERT_TRUE(maybe.has_value());

  ASSERT_EQ(maybe.value(), expected);
}

TEST(Spectrum, LastFit) {
  Spectrum spectrum(10);

  spectrum.allocate({0, 1});

  spectrum.allocate({5, 7});

  const Slice expected = {8, 9};

  const auto maybe = LastFit(spectrum, 2);

  ASSERT_TRUE(maybe.has_value());

  ASSERT_EQ(maybe.value(), expected);
}

TEST(Spectrum, WorstFit) {
  Spectrum spectrum(10);

  spectrum.allocate({0, 1});

  spectrum.allocate({5, 7});

  const Slice expected = {2, 3};

  const auto maybe = WorstFit(spectrum, 2);

  ASSERT_TRUE(maybe.has_value());

  ASSERT_EQ(maybe.value(), expected);
}

// .###...###

TEST(Spectrum, ExternalFragmentation) {
  Spectrum spectrum(10);

  spectrum.allocate({1, 3});

  spectrum.allocate({7, 9});

  const double expected = 0.7;

  const auto absolute_error = 1;

  std::unique_ptr<Fragmentation> fn = std::make_unique<ExternalFragmentation>();

  EXPECT_NEAR(expected, fn->operator()(spectrum), absolute_error);
}
