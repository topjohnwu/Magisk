//===---------------------------------------------------------------------===//
// statistics_test - Unit tests for src/statistics.cc
//===---------------------------------------------------------------------===//

#include "../src/statistics.h"
#include "gtest/gtest.h"

namespace {
TEST(StatisticsTest, Mean) {
  EXPECT_DOUBLE_EQ(benchmark::StatisticsMean({42, 42, 42, 42}), 42.0);
  EXPECT_DOUBLE_EQ(benchmark::StatisticsMean({1, 2, 3, 4}), 2.5);
  EXPECT_DOUBLE_EQ(benchmark::StatisticsMean({1, 2, 5, 10, 10, 14}), 7.0);
}

TEST(StatisticsTest, Median) {
  EXPECT_DOUBLE_EQ(benchmark::StatisticsMedian({42, 42, 42, 42}), 42.0);
  EXPECT_DOUBLE_EQ(benchmark::StatisticsMedian({1, 2, 3, 4}), 2.5);
  EXPECT_DOUBLE_EQ(benchmark::StatisticsMedian({1, 2, 5, 10, 10}), 5.0);
}

TEST(StatisticsTest, StdDev) {
  EXPECT_DOUBLE_EQ(benchmark::StatisticsStdDev({101, 101, 101, 101}), 0.0);
  EXPECT_DOUBLE_EQ(benchmark::StatisticsStdDev({1, 2, 3}), 1.0);
  EXPECT_FLOAT_EQ(benchmark::StatisticsStdDev({1.5, 2.4, 3.3, 4.2, 5.1}),
                  1.42302495);
}

}  // end namespace
