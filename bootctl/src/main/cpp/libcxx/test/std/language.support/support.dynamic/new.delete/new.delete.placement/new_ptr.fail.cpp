// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <new>

// void* operator new(std::size_t, void *);

// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17
// UNSUPPORTED: clang-3.3, clang-3.4, clang-3.5, clang-3.6, clang-3.7, clang-3.8

#include <new>

#include "test_macros.h"

int main ()
{
    char buffer[100];
    ::operator new(4, buffer);  // expected-error {{ignoring return value of function declared with 'nodiscard' attribute}}
}
