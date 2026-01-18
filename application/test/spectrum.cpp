#include <core/spectrum.h>

#include <gtest/gtest.h>

#include <vector>

TEST(Spectrum, AvailableSlices) {
  Spectrum spectrum(10);

  spectrum.allocate({0, 1});

  spectrum.allocate({5, 7});

  std::vector<Slice> expected = {
      {2, 4},
      {8, 9},
  };

  ASSERT_EQ(spectrum.available_slices(), expected);
}

TEST(Spectrum, AvailableFSUs) {
  Spectrum spectrum(10);

  spectrum.allocate({0, 1});

  spectrum.allocate({5, 7});

  const auto expected = 5;

  ASSERT_EQ(spectrum.available(), expected);
}

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

TEST(Spectrum, RandomFit) {
  Spectrum spectrum(10);

  spectrum.allocate({0, 1});

  spectrum.allocate({5, 7});

  ASSERT_TRUE(RandomFit(spectrum, 1).has_value());
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

TEST(Spectrum, ExternalFragmentation) {
  Spectrum spectrum(10);

  spectrum.allocate({1, 3});

  spectrum.allocate({7, 9});

  const double expected = 0.7;

  const auto absolute_error = 1;

  std::unique_ptr<Fragmentation> fn = std::make_unique<ExternalFragmentation>();

  EXPECT_NEAR(expected, (*fn)(spectrum), absolute_error);
}

TEST(Spectrum, EntropyBasedFragmentation) {
  Spectrum spectrum(10);

  spectrum.allocate({0, 2});

  spectrum.allocate({4, 5});

  const double expected = 0.721928095;

  const auto absolute_error = .25;

  std::unique_ptr<Fragmentation> fn =
      std::make_unique<EntropyBasedFragmentation>(1);

  EXPECT_NEAR(expected, (*fn)(spectrum), absolute_error);
}

TEST(Spectrum, AbsoluteFragmentation) {
  Spectrum spectrum(10);

  spectrum.allocate({1, 3});

  spectrum.allocate({7, 9});

  const double expected = 0.25;

  const auto absolute_error = .15;

  std::unique_ptr<Fragmentation> fn = std::make_unique<AbsoluteFragmentation>();

  EXPECT_NEAR(expected, (*fn)(spectrum), absolute_error);
}
