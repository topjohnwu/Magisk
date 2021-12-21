//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <deque>

// deque()
// deque::iterator()

// MODULES_DEFINES: _LIBCPP_ABI_INCOMPLETE_TYPES_IN_DEQUE
#define _LIBCPP_ABI_INCOMPLETE_TYPES_IN_DEQUE
#include <deque>
#include <cassert>

struct A {
  std::deque<A> d;
  std::deque<A>::iterator it;
  std::deque<A>::reverse_iterator it2;
};

int main()
{
  A a;
  assert(a.d.size() == 0);
  a.it = a.d.begin();
  a.it2 = a.d.rend();
}
