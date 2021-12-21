//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11

// XFAIL: gcc-7

// <functional>

// equal_to, not_equal_to, less, et al.

// Test that these types can be constructed w/o an initializer in a constexpr
// context. This is specifically testing gcc.gnu.org/PR83921


#include <functional>
#include "test_macros.h"

template <class T>
constexpr bool test_constexpr_context() {
  std::equal_to<T> eq;
  ((void)eq);
  std::not_equal_to<T> neq;
  ((void)neq);
  std::less<T> l;
  ((void)l);
  std::less_equal<T> le;
  ((void)le);
  std::greater<T> g;
  ((void)g);
  std::greater_equal<T> ge;
  ((void)ge);
  return true;
}

static_assert(test_constexpr_context<int>(), "");
static_assert(test_constexpr_context<void>(), "");


int main() {

}
