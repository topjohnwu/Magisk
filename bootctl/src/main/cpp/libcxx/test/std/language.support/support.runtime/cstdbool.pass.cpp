//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test <cstdbool>

#include <cstdbool>

#ifndef __bool_true_false_are_defined
#error __bool_true_false_are_defined not defined
#endif

#ifdef bool
#error bool should not be defined
#endif

#ifdef true
#error true should not be defined
#endif

#ifdef false
#error false should not be defined
#endif

int main()
{
}
