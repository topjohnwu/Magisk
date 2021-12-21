#ifndef BENCHMARK_REGISTER_H
#define BENCHMARK_REGISTER_H

#include <vector>

#include "check.h"

template <typename T>
void AddRange(std::vector<T>* dst, T lo, T hi, int mult) {
  CHECK_GE(lo, 0);
  CHECK_GE(hi, lo);
  CHECK_GE(mult, 2);

  // Add "lo"
  dst->push_back(lo);

  static const T kmax = std::numeric_limits<T>::max();

  // Now space out the benchmarks in multiples of "mult"
  for (T i = 1; i < kmax / mult; i *= mult) {
    if (i >= hi) break;
    if (i > lo) {
      dst->push_back(i);
    }
  }

  // Add "hi" (if different from "lo")
  if (hi != lo) {
    dst->push_back(hi);
  }
}

#endif  // BENCHMARK_REGISTER_H
