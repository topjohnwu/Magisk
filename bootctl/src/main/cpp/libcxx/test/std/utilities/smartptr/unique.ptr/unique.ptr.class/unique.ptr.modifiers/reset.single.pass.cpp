//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// unique_ptr

// test reset

#include <memory>
#include <cassert>

#include "unique_ptr_test_helper.h"

int main() {
  {
    std::unique_ptr<A> p(new A);
    assert(A::count == 1);
    assert(B::count == 0);
    A* i = p.get();
    assert(i != nullptr);
    p.reset(new B);
    assert(A::count == 1);
    assert(B::count == 1);
  }
  assert(A::count == 0);
  assert(B::count == 0);
  {
    std::unique_ptr<A> p(new B);
    assert(A::count == 1);
    assert(B::count == 1);
    A* i = p.get();
    assert(i != nullptr);
    p.reset(new B);
    assert(A::count == 1);
    assert(B::count == 1);
  }
  assert(A::count == 0);
  assert(B::count == 0);
}
