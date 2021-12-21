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

// class partial_ordering


#include <compare>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

const volatile void* volatile sink;

void test_static_members() {
  DoNotOptimize(&std::partial_ordering::less);
  DoNotOptimize(&std::partial_ordering::equivalent);
  DoNotOptimize(&std::partial_ordering::greater);
  DoNotOptimize(&std::partial_ordering::unordered);
}

void test_signatures() {
  auto& Eq = std::partial_ordering::equivalent;

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
  ASSERT_SAME_TYPE(decltype(Eq <=> 0), std::partial_ordering);
  ASSERT_SAME_TYPE(decltype(0 <=> Eq), std::partial_ordering);
#endif
}

constexpr bool test_conversion() {
  static_assert(std::is_convertible<const std::partial_ordering, std::weak_equality>::value, "");
  { // value == 0
    auto V = std::partial_ordering::equivalent;
    std::weak_equality WV = V;
    assert(WV == 0);
  }
  std::partial_ordering TestCases[] = {
      std::partial_ordering::less,
      std::partial_ordering::greater,
      std::partial_ordering::unordered
  };
  for (auto V : TestCases)
  { // value != 0
    std::weak_equality WV = V;
    assert(WV != 0);
  }
  return true;
}

constexpr bool test_constexpr() {
  auto& Eq = std::partial_ordering::equivalent;
  auto& Less = std::partial_ordering::less;
  auto& Greater = std::partial_ordering::greater;
  auto& Unord = std::partial_ordering::unordered;
  struct {
    std::partial_ordering Value;
    bool ExpectEq;
    bool ExpectNeq;
    bool ExpectLess;
    bool ExpectGreater;
  } TestCases[] = {
      {Eq, true, false, false, false},
      {Less, false, true, true, false},
      {Greater, false, true, false, true},
      {Unord, false, true, false, false}
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
    std::partial_ordering res = (Eq <=> 0);
    ((void)res);
    res = (0 <=> Eq);
    ((void)res);
  }
  enum ExpectRes {
    ER_Greater,
    ER_Less,
    ER_Equiv,
    ER_Unord
  };
  struct {
    std::partial_ordering Value;
    ExpectRes Expect;
  } SpaceshipTestCases[] = {
      {std::partial_ordering::equivalent, ER_Equiv},
      {std::partial_ordering::less, ER_Less},
      {std::partial_ordering::greater, ER_Greater},
      {std::partial_ordering::unordered, ER_Unord}
  };
  for (auto TC : SpaceshipTestCases)
  {
    std::partial_ordering Res = (0 <=> TC.Value);
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
    case ER_Unord:
      assert(Res != 0);
      assert(0 != Res);
      assert((Res < 0) == false);
      assert((Res > 0) == false);
      assert((Res == 0) == false);
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
