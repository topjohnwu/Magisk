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

// template <class T> constexpr T* launder(T* p) noexcept;
// The program is ill-formed if T is a function type or cv void.

// UNSUPPORTED: c++98, c++03, c++11, c++14

#include <new>
#include <cassert>

#include "test_macros.h"

void foo() {}

int main ()
{
    void *p = nullptr;
    (void) std::launder((               void *) nullptr);
    (void) std::launder((const          void *) nullptr);
    (void) std::launder((      volatile void *) nullptr);
    (void) std::launder((const volatile void *) nullptr);  // expected-error-re@new:* 4 {{static_assert failed{{.*}} "can't launder cv-void"}}
    // expected-error@new:* 0-4 {{void pointer argument to '__builtin_launder' is not allowed}}

    (void) std::launder(foo);                              // expected-error-re@new:* 1 {{static_assert failed{{.*}} "can't launder functions"}}
    // expected-error@new:* 0-1 {{function pointer argument to '__builtin_launder' is not allowed}}
}
