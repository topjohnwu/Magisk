// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <variant>

// template <class ...Types>
// constexpr bool
// operator==(variant<Types...> const&, variant<Types...> const&) noexcept;
//
// template <class ...Types>
// constexpr bool
// operator!=(variant<Types...> const&, variant<Types...> const&) noexcept;
//
// template <class ...Types>
// constexpr bool
// operator<(variant<Types...> const&, variant<Types...> const&) noexcept;
//
// template <class ...Types>
// constexpr bool
// operator>(variant<Types...> const&, variant<Types...> const&) noexcept;
//
// template <class ...Types>
// constexpr bool
// operator<=(variant<Types...> const&, variant<Types...> const&) noexcept;
//
// template <class ...Types>
// constexpr bool
// operator>=(variant<Types...> const&, variant<Types...> const&) noexcept;

#include <cassert>
#include <type_traits>
#include <utility>
#include <variant>

#include "test_macros.h"


struct MyBoolExplicit {
  bool value;
  constexpr explicit MyBoolExplicit(bool v) : value(v) {}
  constexpr explicit operator bool() const noexcept { return value; }
};

struct ComparesToMyBoolExplicit {
  int value = 0;
};
inline constexpr MyBoolExplicit operator==(const ComparesToMyBoolExplicit& LHS, const ComparesToMyBoolExplicit& RHS) noexcept {
  return MyBoolExplicit(LHS.value == RHS.value);
}
inline constexpr MyBoolExplicit operator!=(const ComparesToMyBoolExplicit& LHS, const ComparesToMyBoolExplicit& RHS) noexcept {
  return MyBoolExplicit(LHS.value != RHS.value);
}
inline constexpr MyBoolExplicit operator<(const ComparesToMyBoolExplicit& LHS, const ComparesToMyBoolExplicit& RHS) noexcept {
  return MyBoolExplicit(LHS.value < RHS.value);
}
inline constexpr MyBoolExplicit operator<=(const ComparesToMyBoolExplicit& LHS, const ComparesToMyBoolExplicit& RHS) noexcept {
  return MyBoolExplicit(LHS.value <= RHS.value);
}
inline constexpr MyBoolExplicit operator>(const ComparesToMyBoolExplicit& LHS, const ComparesToMyBoolExplicit& RHS) noexcept {
  return MyBoolExplicit(LHS.value > RHS.value);
}
inline constexpr MyBoolExplicit operator>=(const ComparesToMyBoolExplicit& LHS, const ComparesToMyBoolExplicit& RHS) noexcept {
  return MyBoolExplicit(LHS.value >= RHS.value);
}


int main() {
  using V = std::variant<int, ComparesToMyBoolExplicit>;
  V v1(42);
  V v2(101);
  // expected-error-re@variant:* 6 {{static_assert failed {{.*}}"the relational operator does not return a type which is implicitly convertible to bool"}}
  // expected-error@variant:* 6 {{no viable conversion}}
  (void)(v1 == v2); // expected-note {{here}}
  (void)(v1 != v2); // expected-note {{here}}
  (void)(v1 < v2); // expected-note {{here}}
  (void)(v1 <= v2); // expected-note {{here}}
  (void)(v1 > v2); // expected-note {{here}}
  (void)(v1 >= v2); // expected-note {{here}}
}
