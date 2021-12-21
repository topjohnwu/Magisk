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

// UNSUPPORTED: c++98, c++03, c++11, c++14

#include <new>
#include <cassert>

#include "test_macros.h"

constexpr int gi = 5;
constexpr float gf = 8.f;

int main() {
    static_assert(std::launder(&gi) == &gi, "" );
    static_assert(std::launder(&gf) == &gf, "" );

    const int *i = &gi;
    const float *f = &gf;
    static_assert(std::is_same<decltype(i), decltype(std::launder(i))>::value, "");
    static_assert(std::is_same<decltype(f), decltype(std::launder(f))>::value, "");

    assert(std::launder(i) == i);
    assert(std::launder(f) == f);
}
