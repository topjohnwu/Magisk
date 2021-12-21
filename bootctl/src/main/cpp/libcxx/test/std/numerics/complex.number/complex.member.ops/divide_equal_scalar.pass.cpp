//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <complex>

// complex& operator/=(const T& rhs);

#include <complex>
#include <cassert>

template <class T>
void
test()
{
    std::complex<T> c(1);
    assert(c.real() == 1);
    assert(c.imag() == 0);
    c /= 0.5;
    assert(c.real() == 2);
    assert(c.imag() == 0);
    c /= 0.5;
    assert(c.real() == 4);
    assert(c.imag() == 0);
    c /= -0.5;
    assert(c.real() == -8);
    assert(c.imag() == 0);
    c.imag(2);
    c /= 0.5;
    assert(c.real() == -16);
    assert(c.imag() == 4);
}

int main()
{
    test<float>();
    test<double>();
    test<long double>();
}
