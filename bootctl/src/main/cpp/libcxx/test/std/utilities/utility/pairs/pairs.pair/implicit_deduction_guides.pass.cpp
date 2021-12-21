//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: libcpp-no-deduction-guides

// GCC's implementation of class template deduction is still immature and runs
// into issues with libc++. However GCC accepts this code when compiling
// against libstdc++.
// XFAIL: gcc

// <utility>

// Test that the constructors offered by std::pair are formulated
// so they're compatible with implicit deduction guides, or if that's not
// possible that they provide explicit guides to make it work.

#include <utility>
#include <memory>
#include <string>
#include <cassert>

#include "test_macros.h"
#include "archetypes.hpp"


// Overloads
// ---------------
// (1)  pair(const T1&, const T2&) -> pair<T1, T2>
// (2)  explicit pair(const T1&, const T2&) -> pair<T1, T2>
// (3)  pair(pair const& t) -> decltype(t)
// (4)  pair(pair&& t) -> decltype(t)
// (5)  pair(pair<U1, U2> const&) -> pair<U1, U2>
// (6)  explicit pair(pair<U1, U2> const&) -> pair<U1, U2>
// (7)  pair(pair<U1, U2> &&) -> pair<U1, U2>
// (8)  explicit pair(pair<U1, U2> &&) -> pair<U1, U2>
int main()
{
  using E = ExplicitTestTypes::TestType;
  static_assert(!std::is_convertible<E const&, E>::value, "");
  { // Testing (1)
    int const x = 42;
    std::pair t1("abc", x);
    ASSERT_SAME_TYPE(decltype(t1), std::pair<const char*, int>);
  }
  { // Testing (2)
    std::pair p1(E{}, 42);
    ASSERT_SAME_TYPE(decltype(p1), std::pair<E, int>);

    const E t{};
    std::pair p2(t, E{});
    ASSERT_SAME_TYPE(decltype(p2), std::pair<E, E>);
  }
  { // Testing (3, 5)
    std::pair<double, decltype(nullptr)> const p(0.0, nullptr);
    std::pair p1(p);
    ASSERT_SAME_TYPE(decltype(p1), std::pair<double, decltype(nullptr)>);
  }
  { // Testing (3, 6)
    std::pair<E, decltype(nullptr)> const p(E{}, nullptr);
    std::pair p1(p);
    ASSERT_SAME_TYPE(decltype(p1), std::pair<E, decltype(nullptr)>);
  }
  { // Testing (4, 7)
    std::pair<std::string, void*> p("abc", nullptr);
    std::pair p1(std::move(p));
    ASSERT_SAME_TYPE(decltype(p1), std::pair<std::string, void*>);
  }
  { // Testing (4, 8)
    std::pair<std::string, E> p("abc", E{});
    std::pair p1(std::move(p));
    ASSERT_SAME_TYPE(decltype(p1), std::pair<std::string, E>);
  }
}
