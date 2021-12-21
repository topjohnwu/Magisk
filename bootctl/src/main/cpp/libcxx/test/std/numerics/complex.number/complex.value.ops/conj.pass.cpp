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
//   conj(const complex<T>& x);

#include <complex>
#include <cassert>

template <class T>
void
test(const std::complex<T>& z, std::complex<T> x)
{
    assert(conj(z) == x);
}

template <class T>
void
test()
{
    test(std::complex<T>(1, 2), std::complex<T>(1, -2));
    test(std::complex<T>(-1, 2), std::complex<T>(-1, -2));
    test(std::complex<T>(1, -2), std::complex<T>(1, 2));
    test(std::complex<T>(-1, -2), std::complex<T>(-1, 2));
}

int main()
{
    test<float>();
    test<double>();
    test<long double>();
}
