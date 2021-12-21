// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
// REQUIRES: fcoroutines-ts

// RUN: %build -fcoroutines-ts
// RUN: %run

#include <experimental/coroutine>

#ifndef _LIBCPP_VERSION
#error _LIBCPP_VERSION must be defined
#endif

int main()
{
}
