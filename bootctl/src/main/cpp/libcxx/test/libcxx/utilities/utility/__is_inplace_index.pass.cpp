//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// template <class _Tp> using __is_inplace_index

#include <utility>

struct S {};

int main() {
  using I = std::in_place_index_t<0>;
  static_assert( std::__is_inplace_index<I>::value, "");
  static_assert( std::__is_inplace_index<const I>::value, "");
  static_assert( std::__is_inplace_index<const volatile I>::value, "");
  static_assert( std::__is_inplace_index<I&>::value, "");
  static_assert( std::__is_inplace_index<const I&>::value, "");
  static_assert( std::__is_inplace_index<const volatile I&>::value, "");
  static_assert( std::__is_inplace_index<I&&>::value, "");
  static_assert( std::__is_inplace_index<const I&&>::value, "");
  static_assert( std::__is_inplace_index<const volatile I&&>::value, "");
  static_assert(!std::__is_inplace_index<std::in_place_type_t<int>>::value, "");
  static_assert(!std::__is_inplace_index<std::in_place_t>::value, "");
  static_assert(!std::__is_inplace_index<void>::value, "");
  static_assert(!std::__is_inplace_index<int>::value, "");
  static_assert(!std::__is_inplace_index<S>::value, "");
}
