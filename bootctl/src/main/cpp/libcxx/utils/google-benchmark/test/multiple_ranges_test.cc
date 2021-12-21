#include "benchmark/benchmark.h"

#include <cassert>
#include <iostream>
#include <set>
#include <vector>

class MultipleRangesFixture : public ::benchmark::Fixture {
 public:
  MultipleRangesFixture()
      : expectedValues({{1, 3, 5},
                        {1, 3, 8},
                        {1, 3, 15},
                        {2, 3, 5},
                        {2, 3, 8},
                        {2, 3, 15},
                        {1, 4, 5},
                        {1, 4, 8},
                        {1, 4, 15},
                        {2, 4, 5},
                        {2, 4, 8},
                        {2, 4, 15},
                        {1, 7, 5},
                        {1, 7, 8},
                        {1, 7, 15},
                        {2, 7, 5},
                        {2, 7, 8},
                        {2, 7, 15},
                        {7, 6, 3}}) {}

  void SetUp(const ::benchmark::State& state) {
    std::vector<int64_t> ranges = {state.range(0), state.range(1),
                                   state.range(2)};

    assert(expectedValues.find(ranges) != expectedValues.end());

    actualValues.insert(ranges);
  }

  // NOTE: This is not TearDown as we want to check after _all_ runs are
  // complete.
  virtual ~MultipleRangesFixture() {
    assert(actualValues.size() == expectedValues.size());
    if (actualValues.size() != expectedValues.size()) {
      std::cout << "EXPECTED\n";
      for (auto v : expectedValues) {
        std::cout << "{";
        for (int64_t iv : v) {
          std::cout << iv << ", ";
        }
        std::cout << "}\n";
      }
      std::cout << "ACTUAL\n";
      for (auto v : actualValues) {
        std::cout << "{";
        for (int64_t iv : v) {
          std::cout << iv << ", ";
        }
        std::cout << "}\n";
      }
    }
  }

  std::set<std::vector<int64_t>> expectedValues;
  std::set<std::vector<int64_t>> actualValues;
};

BENCHMARK_DEFINE_F(MultipleRangesFixture, Empty)(benchmark::State& state) {
  for (auto _ : state) {
    int64_t product = state.range(0) * state.range(1) * state.range(2);
    for (int64_t x = 0; x < product; x++) {
      benchmark::DoNotOptimize(x);
    }
  }
}

BENCHMARK_REGISTER_F(MultipleRangesFixture, Empty)
    ->RangeMultiplier(2)
    ->Ranges({{1, 2}, {3, 7}, {5, 15}})
    ->Args({7, 6, 3});

void BM_CheckDefaultArgument(benchmark::State& state) {
  // Test that the 'range()' without an argument is the same as 'range(0)'.
  assert(state.range() == state.range(0));
  assert(state.range() != state.range(1));
  for (auto _ : state) {
  }
}
BENCHMARK(BM_CheckDefaultArgument)->Ranges({{1, 5}, {6, 10}});

static void BM_MultipleRanges(benchmark::State& st) {
  for (auto _ : st) {
  }
}
BENCHMARK(BM_MultipleRanges)->Ranges({{5, 5}, {6, 6}});

BENCHMARK_MAIN();
