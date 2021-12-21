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

struct V {
  int member;
};

int main() {
  std::unique_ptr<V[]> p;
  std::unique_ptr<V[]> const& cp = p;

  p->member; // expected-error {{member reference type 'std::unique_ptr<V []>' is not a pointer}}
  // expected-error@-1 {{no member named 'member'}}

  cp->member; // expected-error {{member reference type 'const std::unique_ptr<V []>' is not a pointer}}
              // expected-error@-1 {{no member named 'member'}}
}
