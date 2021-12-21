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

// void* operator new(std::size_t, std::align_val_t);

// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17
// UNSUPPORTED: clang-3.3, clang-3.4, clang-3.5, clang-3.6, clang-3.7, clang-3.8

// REQUIRES: -faligned-allocation
// RUN: %compile %verify -faligned-allocation

#include <new>

int main ()
{
    ::operator new(4, std::align_val_t{4});  // expected-warning {{ignoring return value of function declared with 'nodiscard' attribute}}
}
