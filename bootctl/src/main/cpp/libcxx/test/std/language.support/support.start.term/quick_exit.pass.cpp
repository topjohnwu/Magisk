//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03

// test quick_exit and at_quick_exit

#include <cstdlib>

void f() {}

int main()
{
#ifdef _LIBCPP_HAS_QUICK_EXIT
    std::at_quick_exit(f);
    std::quick_exit(0);
#endif
}
