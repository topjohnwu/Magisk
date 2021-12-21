//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03
// XFAIL: c++11, c++14

// <functional>

#include <functional>
#include <type_traits>

#include "test_macros.h"

struct S : public std::function<void()> { using function::function; };

int main() {
   S f1( [](){} );
   S f2(std::allocator_arg, std::allocator<int>{}, f1);
}
