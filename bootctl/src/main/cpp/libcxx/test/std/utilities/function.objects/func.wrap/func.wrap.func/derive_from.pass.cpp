//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <functional>

// See https://bugs.llvm.org/show_bug.cgi?id=20002

#include <functional>
#include <type_traits>

#include "test_macros.h"

using Fn = std::function<void()>;
struct S : public std::function<void()> { using function::function; };

int main() {
    S s( [](){} );
    S f1( s );
#if TEST_STD_VER <= 14
    S f2(std::allocator_arg, std::allocator<int>{}, s);
#endif
}
