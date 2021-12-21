//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// template <class _Tp> using __is_inplace_type

#include <utility>

struct S {};

int main() {
  using T = std::in_place_type_t<int>;
  static_assert( std::__is_inplace_type<T>::value, "");
  static_assert( std::__is_inplace_type<const T>::value, "");
  static_assert( std::__is_inplace_type<const volatile T>::value, "");
  static_assert( std::__is_inplace_type<T&>::value, "");
  static_assert( std::__is_inplace_type<const T&>::value, "");
  static_assert( std::__is_inplace_type<const volatile T&>::value, "");
  static_assert( std::__is_inplace_type<T&&>::value, "");
  static_assert( std::__is_inplace_type<const T&&>::value, "");
  static_assert( std::__is_inplace_type<const volatile T&&>::value, "");
  static_assert(!std::__is_inplace_type<std::in_place_index_t<0>>::value, "");
  static_assert(!std::__is_inplace_type<std::in_place_t>::value, "");
  static_assert(!std::__is_inplace_type<void>::value, "");
  static_assert(!std::__is_inplace_type<int>::value, "");
  static_assert(!std::__is_inplace_type<S>::value, "");
}
