//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test <cstdarg>

#include <cstdarg>

#include "test_macros.h"

#ifndef va_arg
#error va_arg not defined
#endif

#if TEST_STD_VER >= 11
#  ifndef va_copy
#    error va_copy is not defined when c++ >= 11
#  endif
#endif

#ifndef va_end
#error va_end not defined
#endif

#ifndef va_start
#error va_start not defined
#endif

int main()
{
    std::va_list va;
    ((void)va);
}
