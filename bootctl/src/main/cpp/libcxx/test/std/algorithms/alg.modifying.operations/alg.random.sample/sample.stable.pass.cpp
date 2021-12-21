//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <algorithm>

// template <class PopulationIterator, class SampleIterator, class Distance,
//           class UniformRandomNumberGenerator>
// SampleIterator sample(PopulationIterator first, PopulationIterator last,
//                       SampleIterator out, Distance n,
//                       UniformRandomNumberGenerator &&g);

#include <algorithm>
#include <random>
#include <cassert>

#include "test_iterators.h"

// Stable if and only if PopulationIterator meets the requirements of a
// ForwardIterator type.
template <class PopulationIterator, class SampleIterator>
void test_stability(bool expect_stable) {
  const unsigned kPopulationSize = 100;
  int ia[kPopulationSize];
  for (unsigned i = 0; i < kPopulationSize; ++i)
    ia[i] = i;
  PopulationIterator first(ia);
  PopulationIterator last(ia + kPopulationSize);

  const unsigned kSampleSize = 20;
  int oa[kPopulationSize];
  SampleIterator out(oa);

  std::minstd_rand g;

  const int kIterations = 1000;
  bool unstable = false;
  for (int i = 0; i < kIterations; ++i) {
    std::sample(first, last, out, kSampleSize, g);
    unstable |= !std::is_sorted(oa, oa + kSampleSize);
  }
  assert(expect_stable == !unstable);
}

int main() {
  test_stability<forward_iterator<int *>, output_iterator<int *> >(true);
  test_stability<input_iterator<int *>, random_access_iterator<int *> >(false);
}
