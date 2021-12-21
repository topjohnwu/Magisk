#include <vector>

#include "../src/benchmark_register.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

TEST(AddRangeTest, Simple) {
  std::vector<int> dst;
  AddRange(&dst, 1, 2, 2);
  EXPECT_THAT(dst, testing::ElementsAre(1, 2));
}

TEST(AddRangeTest, Simple64) {
  std::vector<int64_t> dst;
  AddRange(&dst, static_cast<int64_t>(1), static_cast<int64_t>(2), 2);
  EXPECT_THAT(dst, testing::ElementsAre(1, 2));
}

TEST(AddRangeTest, Advanced) {
  std::vector<int> dst;
  AddRange(&dst, 5, 15, 2);
  EXPECT_THAT(dst, testing::ElementsAre(5, 8, 15));
}

TEST(AddRangeTest, Advanced64) {
  std::vector<int64_t> dst;
  AddRange(&dst, static_cast<int64_t>(5), static_cast<int64_t>(15), 2);
  EXPECT_THAT(dst, testing::ElementsAre(5, 8, 15));
}

}  // end namespace
