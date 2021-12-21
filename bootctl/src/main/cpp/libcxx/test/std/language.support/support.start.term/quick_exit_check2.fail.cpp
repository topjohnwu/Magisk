//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03

// test that referencing quick_exit when _LIBCPP_HAS_QUICK_EXIT is not defined
// results in a compile error.

#include <cstdlib>

void f() {}

int main()
{
#ifndef _LIBCPP_HAS_QUICK_EXIT
    std::quick_exit(0);
#else
#error
#endif
}
