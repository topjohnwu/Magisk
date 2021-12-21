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

// class strong_equality


#include <compare>
#include <type_traits>
#include <cassert>

#include "test_macros.h"

const volatile void* volatile sink;

void test_static_members() {
  DoNotOptimize(&std::strong_equality::equal);
  DoNotOptimize(&std::strong_equality::nonequal);
  DoNotOptimize(&std::strong_equality::equivalent);
  DoNotOptimize(&std::strong_equality::nonequivalent);
}

void test_signatures() {
  auto& Eq = std::strong_equality::equivalent;

  ASSERT_NOEXCEPT(Eq == 0);
  ASSERT_NOEXCEPT(0 == Eq);
  ASSERT_NOEXCEPT(Eq != 0);
  ASSERT_NOEXCEPT(0 != Eq);
#ifndef TEST_HAS_NO_SPACESHIP_OPERATOR
  ASSERT_NOEXCEPT(0 <=> Eq);
  ASSERT_NOEXCEPT(Eq <=> 0);
  ASSERT_SAME_TYPE(decltype(Eq <=> 0), std::strong_equality);
  ASSERT_SAME_TYPE(decltype(0 <=> Eq), std::strong_equality);
#endif
}

void test_conversion() {
  constexpr std::weak_equality res = std::strong_equality::equivalent;
  static_assert(res == 0, "");
  static_assert(std::is_convertible<const std::strong_equality&,
      std::weak_equality>::value, "");
  static_assert(res == 0, "expected equal");

  constexpr std::weak_equality neq_res = std::strong_equality::nonequivalent;
  static_assert(neq_res != 0, "expected not equal");
}

constexpr bool test_constexpr() {
  auto& Eq = std::strong_equality::equal;
  auto& NEq = std::strong_equality::nonequal;
  auto& Equiv = std::strong_equality::equivalent;
  auto& NEquiv = std::strong_equality::nonequivalent;
  assert((Eq == 0) == true);
  assert((0 == Eq) == true);
  assert((Equiv == 0) == true);
  assert((0 == Equiv) == true);
  assert((NEq == 0) == false);
  assert((0 == NEq) == false);
  assert((NEquiv == 0) == false);
  assert((0 == NEquiv) == false);

  assert((Eq != 0) == false);
  assert((0 != Eq) == false);
  assert((Equiv != 0) == false);
  assert((0 != Equiv) == false);
  assert((NEq != 0) == true);
  assert((0 != NEq) == true);
  assert((NEquiv != 0) == true);
  assert((0 != NEquiv) == true);

#ifndef TEST_HAS_NO_SPACESHIP_OPERATOR
  std::strong_equality res = (Eq <=> 0);
  ((void)res);
  res = (0 <=> Eq);
  ((void)res);
#endif

  return true;
}

int main() {
  test_static_members();
  test_signatures();
  test_conversion();
  static_assert(test_constexpr(), "constexpr test failed");
}
