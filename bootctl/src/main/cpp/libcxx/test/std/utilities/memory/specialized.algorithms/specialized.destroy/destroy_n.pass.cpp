//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <memory>

// template <class ForwardIt, class Size>
// ForwardIt destroy_n(ForwardIt, Size s);

#include <memory>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

struct Counted {
  static int count;
  static void reset() { count = 0; }
  Counted() { ++count; }
  Counted(Counted const&) { ++count; }
  ~Counted() { --count; }
  friend void operator&(Counted) = delete;
};
int Counted::count = 0;

int main()
{
    using It = forward_iterator<Counted*>;
    const int N = 5;
    alignas(Counted) char pool[sizeof(Counted)*N] = {};
    Counted* p = (Counted*)pool;
    std::uninitialized_fill(p, p+N, Counted());
    assert(Counted::count == 5);
    Counted* np = std::destroy_n(p, 1);
    assert(np == p+1);
    assert(Counted::count == 4);
    p += 1;
    It it = std::destroy_n(It(p), 4);
    assert(it == It(p+4));
    assert(Counted::count == 0);
}
