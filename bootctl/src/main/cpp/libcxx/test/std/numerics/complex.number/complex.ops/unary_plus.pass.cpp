//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <complex>

// template<class T>
//   complex<T>
//   operator+(const complex<T>&);

#include <complex>
#include <cassert>

template <class T>
void
test()
{
    std::complex<T> z(1.5, 2.5);
    assert(z.real() == 1.5);
    assert(z.imag() == 2.5);
    std::complex<T> c = +z;
    assert(c.real() == 1.5);
    assert(c.imag() == 2.5);
}

int main()
{
    test<float>();
    test<double>();
    test<long double>();
}
