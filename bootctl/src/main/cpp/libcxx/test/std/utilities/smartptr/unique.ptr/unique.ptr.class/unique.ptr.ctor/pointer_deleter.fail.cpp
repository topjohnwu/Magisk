//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Without rvalue references it is impossible to detect when a rvalue deleter
// is given.
// XFAIL: c++98, c++03

// <memory>

// unique_ptr

// unique_ptr<T, const D&>(pointer, D()) should not compile

#include <memory>

struct Deleter {
  void operator()(int* p) const { delete p; }
};

int main() {
  // expected-error@+1 {{call to deleted constructor of 'std::unique_ptr<int, const Deleter &>}}
  std::unique_ptr<int, const Deleter&> s((int*)nullptr, Deleter());
}
