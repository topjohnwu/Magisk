//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <complex>

// constexpr complex(const T& re = T(), const T& im = T());

#include <complex>
#include <cassert>

#include "test_macros.h"

template <class T>
void
test()
{
    {
    const std::complex<T> c;
    assert(c.real() == 0);
    assert(c.imag() == 0);
    }
    {
    const std::complex<T> c = 7.5;
    assert(c.real() == 7.5);
    assert(c.imag() == 0);
    }
    {
    const std::complex<T> c(8.5);
    assert(c.real() == 8.5);
    assert(c.imag() == 0);
    }
    {
    const std::complex<T> c(10.5, -9.5);
    assert(c.real() == 10.5);
    assert(c.imag() == -9.5);
    }
#if TEST_STD_VER >= 11
    {
    constexpr std::complex<T> c;
    static_assert(c.real() == 0, "");
    static_assert(c.imag() == 0, "");
    }
    {
    constexpr std::complex<T> c = 7.5;
    static_assert(c.real() == 7.5, "");
    static_assert(c.imag() == 0, "");
    }
    {
    constexpr std::complex<T> c(8.5);
    static_assert(c.real() == 8.5, "");
    static_assert(c.imag() == 0, "");
    }
    {
    constexpr std::complex<T> c(10.5, -9.5);
    static_assert(c.real() == 10.5, "");
    static_assert(c.imag() == -9.5, "");
    }
#endif
}

int main()
{
    test<float>();
    test<double>();
    test<long double>();
}
