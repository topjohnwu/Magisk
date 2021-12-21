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
//   operator*(const complex<T>& lhs, const T& rhs);

#include <complex>
#include <cassert>

template <class T>
void
test(const std::complex<T>& lhs, const T& rhs, std::complex<T> x)
{
    assert(lhs * rhs == x);
}

template <class T>
void
test()
{
    std::complex<T> lhs(1.5, 2.5);
    T rhs(1.5);
    std::complex<T>   x(2.25, 3.75);
    test(lhs, rhs, x);
}

int main()
{
    test<float>();
    test<double>();
    test<long double>();
}
