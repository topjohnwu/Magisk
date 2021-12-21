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

// test op[](size_t)

#include <memory>
#include <cassert>

int main() {
  std::unique_ptr<int> p(new int[3]);
  std::unique_ptr<int> const& cp = p;
  p[0];  // expected-error {{type 'std::unique_ptr<int>' does not provide a subscript operator}}
  cp[1]; // expected-error {{type 'const std::unique_ptr<int>' does not provide a subscript operator}}
}
