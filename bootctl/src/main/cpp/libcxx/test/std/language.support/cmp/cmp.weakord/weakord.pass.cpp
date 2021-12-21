//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <compare>

// class weak_ordering


#include <compare>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

const volatile void* volatile sink;

void test_static_members() {
  DoNotOptimize(&std::weak_ordering::less);
  DoNotOptimize(&std::weak_ordering::equivalent);
  DoNotOptimize(&std::weak_ordering::greater);
}

void test_signatures() {
  auto& Eq = std::weak_ordering::equivalent;

  ASSERT_NOEXCEPT(Eq == 0);
  ASSERT_NOEXCEPT(0 == Eq);
  ASSERT_NOEXCEPT(Eq != 0);
  ASSERT_NOEXCEPT(0 != Eq);
  ASSERT_NOEXCEPT(0 < Eq);
  ASSERT_NOEXCEPT(Eq < 0);
  ASSERT_NOEXCEPT(0 <= Eq);
  ASSERT_NOEXCEPT(Eq <= 0);
  ASSERT_NOEXCEPT(0 > Eq);
  ASSERT_NOEXCEPT(Eq > 0);
  ASSERT_NOEXCEPT(0 >= Eq);
  ASSERT_NOEXCEPT(Eq >= 0);
#ifndef TEST_HAS_NO_SPACESHIP_OPERATOR
  ASSERT_NOEXCEPT(0 <=> Eq);
  ASSERT_NOEXCEPT(Eq <=> 0);
  ASSERT_SAME_TYPE(decltype(Eq <=> 0), std::weak_ordering);
  ASSERT_SAME_TYPE(decltype(0 <=> Eq), std::weak_ordering);
#endif
}

constexpr bool test_conversion() {
  static_assert(std::is_convertible<const std::weak_ordering&,
      std::weak_equality>::value, "");
  { // value == 0
    auto V = std::weak_ordering::equivalent;
    std::weak_equality WV = V;
    assert(WV == 0);
  }
  std::weak_ordering WeakTestCases[] = {
      std::weak_ordering::less,
      std::weak_ordering::greater,
  };
  for (auto V : WeakTestCases)
  { // value != 0
    std::weak_equality WV = V;
    assert(WV != 0);
  }
  static_assert(std::is_convertible<const std::weak_ordering&,
      std::partial_ordering>::value, "");
  { // value == 0
    auto V = std::weak_ordering::equivalent;
    std::partial_ordering WV = V;
    assert(WV == 0);
  }
  { // value < 0
    auto V = std::weak_ordering::less;
    std::partial_ordering WV = V;
    assert(WV < 0);
  }
  { // value > 0
    auto V = std::weak_ordering::greater;
    std::partial_ordering WV = V;
    assert(WV > 0);
  }
  return true;
}

constexpr bool test_constexpr() {
  auto& Eq = std::weak_ordering::equivalent;
  auto& Less = std::weak_ordering::less;
  auto& Greater = std::weak_ordering::greater;
  struct {
    std::weak_ordering Value;
    bool ExpectEq;
    bool ExpectNeq;
    bool ExpectLess;
    bool ExpectGreater;
  } TestCases[] = {
      {Eq, true, false, false, false},
      {Less, false, true, true, false},
      {Greater, false, true, false, true},
  };
  for (auto TC : TestCases) {
    auto V = TC.Value;
    assert((V == 0) == TC.ExpectEq);
    assert((0 == V) == TC.ExpectEq);
    assert((V != 0) == TC.ExpectNeq);
    assert((0 != V) == TC.ExpectNeq);

    assert((V < 0) == TC.ExpectLess);
    assert((V > 0) == TC.ExpectGreater);
    assert((V <= 0) == (TC.ExpectLess || TC.ExpectEq));
    assert((V >= 0) == (TC.ExpectGreater || TC.ExpectEq));

    assert((0 < V) == TC.ExpectGreater);
    assert((0 > V) == TC.ExpectLess);
    assert((0 <= V) == (TC.ExpectGreater || TC.ExpectEq));
    assert((0 >= V) == (TC.ExpectLess || TC.ExpectEq));
  }
#ifndef TEST_HAS_NO_SPACESHIP_OPERATOR
  {
    std::weak_ordering res = (Eq <=> 0);
    ((void)res);
    res = (0 <=> Eq);
    ((void)res);
  }
  enum ExpectRes {
    ER_Greater,
    ER_Less,
    ER_Equiv
  };
  struct {
    std::weak_ordering Value;
    ExpectRes Expect;
  } SpaceshipTestCases[] = {
      {std::weak_ordering::equivalent, ER_Equiv},
      {std::weak_ordering::less, ER_Less},
      {std::weak_ordering::greater, ER_Greater},
  };
  for (auto TC : SpaceshipTestCases)
  {
    std::weak_ordering Res = (0 <=> TC.Value);
    switch (TC.Expect) {
    case ER_Equiv:
      assert(Res == 0);
      assert(0 == Res);
      break;
    case ER_Less:
      assert(Res < 0);
      break;
    case ER_Greater:
      assert(Res > 0);
      break;
    }
  }
#endif

  return true;
}

int main() {
  test_static_members();
  test_signatures();
  static_assert(test_conversion(), "conversion test failed");
  static_assert(test_constexpr(), "constexpr test failed");
}
