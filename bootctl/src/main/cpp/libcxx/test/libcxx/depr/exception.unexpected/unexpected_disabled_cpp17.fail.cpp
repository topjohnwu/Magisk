//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// test unexpected

#include <exception>

void f() {}

int main() {
  using T = std::unexpected_handler; // expected-error {{no type named 'unexpected_handler' in namespace 'std'}}
  std::unexpected(); // expected-error {{no member named 'unexpected' in namespace 'std'}}
  std::get_unexpected(); // expected-error {{no member named 'get_unexpected' in namespace 'std'}}
  std::set_unexpected(f); // expected-error {{no type named 'set_unexpected' in namespace 'std'}}
}
