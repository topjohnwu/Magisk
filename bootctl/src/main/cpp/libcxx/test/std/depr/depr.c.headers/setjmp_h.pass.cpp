//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test <setjmp.h>

#include <setjmp.h>
#include <type_traits>

#ifndef setjmp
#error setjmp not defined
#endif

int main()
{
    jmp_buf jb;
    ((void)jb); // Prevent unused warning
    static_assert((std::is_same<decltype(longjmp(jb, 0)), void>::value),
                  "std::is_same<decltype(longjmp(jb, 0)), void>::value");
}
