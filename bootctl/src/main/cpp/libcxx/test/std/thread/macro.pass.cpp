//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads

// <thread>

// #define __STDCPP_THREADS__ __cplusplus

#include <thread>

int main()
{
#ifndef __STDCPP_THREADS__
#error __STDCPP_THREADS__ is not defined
#endif
}
