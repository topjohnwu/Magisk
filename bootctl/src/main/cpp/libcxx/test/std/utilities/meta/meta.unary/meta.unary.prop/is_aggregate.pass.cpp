//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <type_traits>

// template <class T> struct is_aggregate;
// template <class T> constexpr bool is_aggregate_v = is_aggregate<T>::value;

#include <type_traits>
#include "test_macros.h"

template <class T>
void test_true()
{
#if !defined(_LIBCPP_HAS_NO_IS_AGGREGATE)
    static_assert( std::is_aggregate<T>::value, "");
    static_assert( std::is_aggregate<const T>::value, "");
    static_assert( std::is_aggregate<volatile T>::value, "");
    static_assert( std::is_aggregate<const volatile T>::value, "");
    static_assert( std::is_aggregate_v<T>, "");
    static_assert( std::is_aggregate_v<const T>, "");
    static_assert( std::is_aggregate_v<volatile T>, "");
    static_assert( std::is_aggregate_v<const volatile T>, "");
#endif
}

template <class T>
void test_false()
{
#if !defined(_LIBCPP_HAS_NO_IS_AGGREGATE)
    static_assert(!std::is_aggregate<T>::value, "");
    static_assert(!std::is_aggregate<const T>::value, "");
    static_assert(!std::is_aggregate<volatile T>::value, "");
    static_assert(!std::is_aggregate<const volatile T>::value, "");
    static_assert(!std::is_aggregate_v<T>, "");
    static_assert(!std::is_aggregate_v<const T>, "");
    static_assert(!std::is_aggregate_v<volatile T>, "");
    static_assert(!std::is_aggregate_v<const volatile T>, "");
#endif
}

struct Aggregate {};
struct HasCons { HasCons(int); };
struct HasPriv {
  void PreventUnusedPrivateMemberWarning();
private:
  int x;
};
struct Union { int x; void* y; };


int main ()
{
  {
    test_false<void>();
    test_false<int>();
    test_false<void*>();
    test_false<void()>();
    test_false<void() const>();
    test_false<void(Aggregate::*)(int) const>();
    test_false<Aggregate&>();
    test_false<HasCons>();
    test_false<HasPriv>();
  }
  {
    test_true<Aggregate>();
    test_true<Aggregate[]>();
    test_true<Aggregate[42][101]>();
    test_true<Union>();
  }
}
