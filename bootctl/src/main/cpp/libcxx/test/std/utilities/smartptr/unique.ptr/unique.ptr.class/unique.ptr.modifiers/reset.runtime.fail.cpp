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
    std::unique_ptr<A[]> p;
    p.reset(static_cast<B*>(nullptr)); // expected-error {{no matching member function for call to 'reset'}}
  }
  {
    std::unique_ptr<int[]> p;
    p.reset(static_cast<const int*>(nullptr)); // expected-error {{no matching member function for call to 'reset'}}
  }
}
