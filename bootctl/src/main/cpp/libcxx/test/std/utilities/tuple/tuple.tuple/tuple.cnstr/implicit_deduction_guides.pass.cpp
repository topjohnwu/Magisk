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

// <tuple>

// Test that the constructors offered by std::tuple are formulated
// so they're compatible with implicit deduction guides, or if that's not
// possible that they provide explicit guides to make it work.

#include <tuple>
#include <memory>
#include <cassert>

#include "test_macros.h"
#include "archetypes.hpp"


// Overloads
//  using A = Allocator
//  using AT = std::allocator_arg_t
// ---------------
// (1)  tuple(const Types&...) -> tuple<Types...>
// (2)  explicit tuple(const Types&...) -> tuple<Types...>
// (3)  tuple(AT, A const&, Types const&...) -> tuple<Types...>
// (4)  explicit tuple(AT, A const&, Types const&...) -> tuple<Types...>
// (5)  tuple(tuple const& t) -> decltype(t)
// (6)  tuple(tuple&& t) -> decltype(t)
// (7)  tuple(AT, A const&, tuple const& t) -> decltype(t)
// (8)  tuple(AT, A const&, tuple&& t) -> decltype(t)
void test_primary_template()
{
  const std::allocator<int> A;
  const auto AT = std::allocator_arg;
  { // Testing (1)
    int x = 101;
    std::tuple t1(42);
    ASSERT_SAME_TYPE(decltype(t1), std::tuple<int>);
    std::tuple t2(x, 0.0, nullptr);
    ASSERT_SAME_TYPE(decltype(t2), std::tuple<int, double, decltype(nullptr)>);
  }
  { // Testing (2)
    using T = ExplicitTestTypes::TestType;
    static_assert(!std::is_convertible<T const&, T>::value, "");

    std::tuple t1(T{});
    ASSERT_SAME_TYPE(decltype(t1), std::tuple<T>);

    const T v{};
    std::tuple t2(T{}, 101l, v);
    ASSERT_SAME_TYPE(decltype(t2), std::tuple<T, long, T>);
  }
  { // Testing (3)
    int x = 101;
    std::tuple t1(AT, A, 42);
    ASSERT_SAME_TYPE(decltype(t1), std::tuple<int>);

    std::tuple t2(AT, A, 42, 0.0, x);
    ASSERT_SAME_TYPE(decltype(t2), std::tuple<int, double, int>);
  }
  { // Testing (4)
    using T = ExplicitTestTypes::TestType;
    static_assert(!std::is_convertible<T const&, T>::value, "");

    std::tuple t1(AT, A, T{});
    ASSERT_SAME_TYPE(decltype(t1), std::tuple<T>);

    const T v{};
    std::tuple t2(AT, A, T{}, 101l, v);
    ASSERT_SAME_TYPE(decltype(t2), std::tuple<T, long, T>);
  }
  { // Testing (5)
    using Tup = std::tuple<int, decltype(nullptr)>;
    const Tup t(42, nullptr);

    std::tuple t1(t);
    ASSERT_SAME_TYPE(decltype(t1), Tup);
  }
  { // Testing (6)
    using Tup = std::tuple<void*, unsigned, char>;
    std::tuple t1(Tup(nullptr, 42, 'a'));
    ASSERT_SAME_TYPE(decltype(t1), Tup);
  }
  { // Testing (7)
    using Tup = std::tuple<int, decltype(nullptr)>;
    const Tup t(42, nullptr);

    std::tuple t1(AT, A, t);
    ASSERT_SAME_TYPE(decltype(t1), Tup);
  }
  { // Testing (8)
    using Tup = std::tuple<void*, unsigned, char>;
    std::tuple t1(AT, A, Tup(nullptr, 42, 'a'));
    ASSERT_SAME_TYPE(decltype(t1), Tup);
  }
}

// Overloads
//  using A = Allocator
//  using AT = std::allocator_arg_t
// ---------------
// (1)  tuple() -> tuple<>
// (2)  tuple(AT, A const&) -> tuple<>
// (3)  tuple(tuple const&) -> tuple<>
// (4)  tuple(tuple&&) -> tuple<>
// (5)  tuple(AT, A const&, tuple const&) -> tuple<>
// (6)  tuple(AT, A const&, tuple&&) -> tuple<>
void test_empty_specialization()
{
  std::allocator<int> A;
  const auto AT = std::allocator_arg;
  { // Testing (1)
    std::tuple t1{};
    ASSERT_SAME_TYPE(decltype(t1), std::tuple<>);
  }
  { // Testing (2)
    std::tuple t1{AT, A};
    ASSERT_SAME_TYPE(decltype(t1), std::tuple<>);
  }
  { // Testing (3)
    const std::tuple<> t{};
    std::tuple t1(t);
    ASSERT_SAME_TYPE(decltype(t1), std::tuple<>);
  }
  { // Testing (4)
    std::tuple t1(std::tuple<>{});
    ASSERT_SAME_TYPE(decltype(t1), std::tuple<>);
  }
  { // Testing (5)
    const std::tuple<> t{};
    std::tuple t1(AT, A, t);
    ASSERT_SAME_TYPE(decltype(t1), std::tuple<>);
  }
  { // Testing (6)
    std::tuple t1(AT, A, std::tuple<>{});
    ASSERT_SAME_TYPE(decltype(t1), std::tuple<>);
  }
}

int main() {
  test_primary_template();
  test_empty_specialization();
}
