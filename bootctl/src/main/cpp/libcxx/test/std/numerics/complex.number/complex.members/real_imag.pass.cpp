//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <complex>

// void real(T val);
// void imag(T val);

#include <complex>
#include <cassert>

#include "test_macros.h"

template <class T>
void
test_constexpr()
{
#if TEST_STD_VER > 11
    constexpr std::complex<T> c1;
    static_assert(c1.real() == 0, "");
    static_assert(c1.imag() == 0, "");
    constexpr std::complex<T> c2(3);
    static_assert(c2.real() == 3, "");
    static_assert(c2.imag() == 0, "");
    constexpr std::complex<T> c3(3, 4);
    static_assert(c3.real() == 3, "");
    static_assert(c3.imag() == 4, "");
#endif
}

template <class T>
void
test()
{
    std::complex<T> c;
    assert(c.real() == 0);
    assert(c.imag() == 0);
    c.real(3.5);
    assert(c.real() == 3.5);
    assert(c.imag() == 0);
    c.imag(4.5);
    assert(c.real() == 3.5);
    assert(c.imag() == 4.5);
    c.real(-4.5);
    assert(c.real() == -4.5);
    assert(c.imag() == 4.5);
    c.imag(-5.5);
    assert(c.real() == -4.5);
    assert(c.imag() == -5.5);

    test_constexpr<T> ();
}

int main()
{
    test<float>();
    test<double>();
    test<long double>();
    test_constexpr<int> ();
}
