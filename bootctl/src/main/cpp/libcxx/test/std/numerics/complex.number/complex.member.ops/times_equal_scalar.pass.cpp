//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <complex>

// complex& operator*=(const T& rhs);

#include <complex>
#include <cassert>

template <class T>
void
test()
{
    std::complex<T> c(1);
    assert(c.real() == 1);
    assert(c.imag() == 0);
    c *= 1.5;
    assert(c.real() == 1.5);
    assert(c.imag() == 0);
    c *= 1.5;
    assert(c.real() == 2.25);
    assert(c.imag() == 0);
    c *= -1.5;
    assert(c.real() == -3.375);
    assert(c.imag() == 0);
    c.imag(2);
    c *= 1.5;
    assert(c.real() == -5.0625);
    assert(c.imag() == 3);
}

int main()
{
    test<float>();
    test<double>();
    test<long double>();
}
