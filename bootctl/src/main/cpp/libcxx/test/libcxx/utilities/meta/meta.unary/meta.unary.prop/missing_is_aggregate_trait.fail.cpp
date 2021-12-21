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

int main ()
{
#ifdef _LIBCPP_HAS_NO_IS_AGGREGATE
  // This should not compile when _LIBCPP_HAS_NO_IS_AGGREGATE is defined.
  bool b = __is_aggregate(void);
  ((void)b);
#else
#error Forcing failure...
#endif
}
