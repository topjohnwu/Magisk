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
//   operator/(const T& lhs, const complex<T>& rhs);

#include <complex>
#include <cassert>

template <class T>
void
test(const T& lhs, const std::complex<T>& rhs, std::complex<T> x)
{
    assert(lhs / rhs == x);
}

template <class T>
void
test()
{
    T lhs(-8.5);
    std::complex<T> rhs(1.5, 2.5);
    std::complex<T>   x(-1.5, 2.5);
    test(lhs, rhs, x);
}

int main()
{
    test<float>();
    test<double>();
    test<long double>();
}
