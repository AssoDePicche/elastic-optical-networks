#include <core/spectrum.h>
#include <gtest/gtest.h>

#include <vector>

TEST(Spectrum, AvailableSlices) {
  core::Spectrum spectrum(10);

  spectrum.allocate({0, 1});

  spectrum.allocate({5, 7});

  std::vector<core::Slice> expected = {
      {2, 4},
      {8, 9},
  };

  ASSERT_EQ(spectrum.available_slices(), expected);
}

TEST(Spectrum, AvailableFSUs) {
  core::Spectrum spectrum(10);

  spectrum.allocate({0, 1});

  spectrum.allocate({5, 7});

  const auto expected = 5;

  ASSERT_EQ(spectrum.available(), expected);
}

TEST(Spectrum, BestFit) {
  core::Spectrum spectrum(10);

  spectrum.allocate({0, 1});

  spectrum.allocate({5, 7});

  const core::Slice expected = {8, 9};

  const auto maybe = core::BestFit(spectrum, 2);

  ASSERT_TRUE(maybe.has_value());

  ASSERT_EQ(maybe.value(), expected);
}

TEST(Spectrum, FirstFit) {
  core::Spectrum spectrum(10);

  spectrum.allocate({0, 1});

  spectrum.allocate({5, 7});

  const core::Slice expected = {2, 3};

  const auto maybe = FirstFit(spectrum, 2);

  ASSERT_TRUE(maybe.has_value());

  ASSERT_EQ(maybe.value(), expected);
}

TEST(Spectrum, LastFit) {
  core::Spectrum spectrum(10);

  spectrum.allocate({0, 1});

  spectrum.allocate({5, 7});

  const core::Slice expected = {8, 9};

  const auto maybe = core::LastFit(spectrum, 2);

  ASSERT_TRUE(maybe.has_value());

  ASSERT_EQ(maybe.value(), expected);
}

TEST(Spectrum, RandomFit) {
  core::Spectrum spectrum(10);

  spectrum.allocate({0, 1});

  spectrum.allocate({5, 7});

  ASSERT_TRUE(core::RandomFit(spectrum, 1).has_value());
}

TEST(Spectrum, WorstFit) {
  core::Spectrum spectrum(10);

  spectrum.allocate({0, 1});

  spectrum.allocate({5, 7});

  const core::Slice expected = {2, 3};

  const auto maybe = core::WorstFit(spectrum, 2);

  ASSERT_TRUE(maybe.has_value());

  ASSERT_EQ(maybe.value(), expected);
}

TEST(Spectrum, ExternalFragmentation) {
  core::Spectrum spectrum(10);

  spectrum.allocate({1, 3});

  spectrum.allocate({7, 9});

  const double expected = 0.7;

  const auto absolute_error = 1;

  std::unique_ptr<core::Fragmentation> fn =
      std::make_unique<core::ExternalFragmentation>();

  EXPECT_NEAR(expected, (*fn)(spectrum), absolute_error);
}

TEST(Spectrum, EntropyBasedFragmentation) {
  core::Spectrum spectrum(10);

  spectrum.allocate({0, 2});

  spectrum.allocate({4, 5});

  const double expected = 0.721928095;

  const auto absolute_error = .25;

  std::unique_ptr<core::Fragmentation> fn =
      std::make_unique<core::EntropyBasedFragmentation>(1);

  EXPECT_NEAR(expected, (*fn)(spectrum), absolute_error);
}

TEST(Spectrum, AbsoluteFragmentation) {
  core::Spectrum spectrum(10);

  spectrum.allocate({1, 3});

  spectrum.allocate({7, 9});

  const double expected = 0.25;

  const auto absolute_error = .15;

  std::unique_ptr<core::Fragmentation> fn =
      std::make_unique<core::AbsoluteFragmentation>();

  EXPECT_NEAR(expected, (*fn)(spectrum), absolute_error);
}
