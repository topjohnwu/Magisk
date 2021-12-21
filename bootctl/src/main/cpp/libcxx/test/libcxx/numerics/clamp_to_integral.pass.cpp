//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// __clamp_to_integral<IntT>(RealT)

// Test the conversion function that truncates floating point types to the
// closest representable value for the specified integer type, or
// numeric_limits<IntT>::max()/min() if the value isn't representable.

#include <limits>
#include <cassert>
#include <cmath>

template <class IntT>
void test() {
  typedef std::numeric_limits<IntT> Lim;
  const bool MaxIsRepresentable = sizeof(IntT) < 8;
  const bool IsSigned = std::is_signed<IntT>::value;
  struct TestCase {
    double Input;
    IntT Expect;
    bool IsRepresentable;
  } TestCases[] = {
      {0, 0, true},
      {1, 1, true},
      {IsSigned ? static_cast<IntT>(-1) : 0,
       IsSigned ? static_cast<IntT>(-1) : 0, true},
      {Lim::lowest(), Lim::lowest(), true},
      {static_cast<double>(Lim::max()), Lim::max(), MaxIsRepresentable},
      {static_cast<double>(Lim::max()) + 1, Lim::max(), false},
      {static_cast<double>(Lim::max()) + 1024, Lim::max(), false},
      {nextafter(static_cast<double>(Lim::max()), INFINITY), Lim::max(), false},
  };
  for (TestCase TC : TestCases) {
    auto res = std::__clamp_to_integral<IntT>(TC.Input);
    assert(res == TC.Expect);
    if (TC.IsRepresentable) {
      auto other = static_cast<IntT>(std::trunc(TC.Input));
      assert(res == other);
    } else
      assert(res == Lim::min() || res == Lim::max());
  }
}

template <class IntT>
void test_float() {
  typedef std::numeric_limits<IntT> Lim;
  const bool MaxIsRepresentable = sizeof(IntT) < 4;
  ((void)MaxIsRepresentable);
  const bool IsSigned = std::is_signed<IntT>::value;
  struct TestCase {
    float Input;
    IntT Expect;
    bool IsRepresentable;
  } TestCases[] = {
      {0, 0, true},
      {1, 1, true},
      {IsSigned ? static_cast<IntT>(-1) : 0,
       IsSigned ? static_cast<IntT>(-1) : 0, true},
      {Lim::lowest(), Lim::lowest(), true},
      {static_cast<float>(Lim::max()), Lim::max(), MaxIsRepresentable },
       {nextafter(static_cast<float>(Lim::max()), INFINITY), Lim::max(), false},
  };
  for (TestCase TC : TestCases) {
    auto res = std::__clamp_to_integral<IntT>(TC.Input);
    assert(res == TC.Expect);
    if (TC.IsRepresentable) {
      auto other = static_cast<IntT>(std::trunc(TC.Input));
      assert(res == other);
    } else
      assert(res == Lim::min() || res == Lim::max());
  }
}

int main() {
  test<short>();
  test<unsigned short>();
  test<int>();
  test<unsigned>();
  test<long long>();
  test<unsigned long long>();
  test_float<short>();
  test_float<int>();
  test_float<long long>();
}
