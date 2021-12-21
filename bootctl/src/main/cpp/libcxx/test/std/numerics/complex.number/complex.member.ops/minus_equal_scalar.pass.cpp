//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <complex>

// complex& operator-=(const T& rhs);

#include <complex>
#include <cassert>

template <class T>
void
test()
{
    std::complex<T> c;
    assert(c.real() == 0);
    assert(c.imag() == 0);
    c -= 1.5;
    assert(c.real() == -1.5);
    assert(c.imag() == 0);
    c -= 1.5;
    assert(c.real() == -3);
    assert(c.imag() == 0);
    c -= -1.5;
    assert(c.real() == -1.5);
    assert(c.imag() == 0);
}

int main()
{
    test<float>();
    test<double>();
    test<long double>();
}
