//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <complex>

// complex& operator/=(const complex& rhs);

#include <complex>
#include <cassert>

template <class T>
void
test()
{
    std::complex<T> c(-4, 7.5);
    const std::complex<T> c2(1.5, 2.5);
    assert(c.real() == -4);
    assert(c.imag() == 7.5);
    c /= c2;
    assert(c.real() == 1.5);
    assert(c.imag() == 2.5);
    c /= c2;
    assert(c.real() == 1);
    assert(c.imag() == 0);

    std::complex<T> c3;

    c3 = c;
    std::complex<int> ic (1,1);
    c3 /= ic;
    assert(c3.real() ==  0.5);
    assert(c3.imag() == -0.5);

    c3 = c;
    std::complex<float> fc (1,1);
    c3 /= fc;
    assert(c3.real() ==  0.5);
    assert(c3.imag() == -0.5);

}

int main()
{
    test<float>();
    test<double>();
    test<long double>();
}
