//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <complex>

// complex& operator=(const complex&);
// template<class X> complex& operator= (const complex<X>&);

#include <complex>
#include <cassert>

template <class T, class X>
void
test()
{
    std::complex<T> c;
    assert(c.real() == 0);
    assert(c.imag() == 0);
    std::complex<T> c2(1.5, 2.5);
    c = c2;
    assert(c.real() == 1.5);
    assert(c.imag() == 2.5);
    std::complex<X> c3(3.5, -4.5);
    c = c3;
    assert(c.real() == 3.5);
    assert(c.imag() == -4.5);
}

int main()
{
    test<float, float>();
    test<float, double>();
    test<float, long double>();

    test<double, float>();
    test<double, double>();
    test<double, long double>();

    test<long double, float>();
    test<long double, double>();
    test<long double, long double>();
}
