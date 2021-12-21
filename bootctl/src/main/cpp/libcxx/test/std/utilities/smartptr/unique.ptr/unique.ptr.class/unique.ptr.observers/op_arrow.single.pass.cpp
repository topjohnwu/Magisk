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

// test op->()

#include <memory>
#include <cassert>

struct A {
  int i_;

  A() : i_(7) {}
};

int main() {
  std::unique_ptr<A> p(new A);
  assert(p->i_ == 7);
}
