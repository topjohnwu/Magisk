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
//   pow(const complex<T>& x, const T& y);

#include <complex>
#include <cassert>

#include "../cases.h"

template <class T>
void
test(const std::complex<T>& a, const T& b, std::complex<T> x)
{
    std::complex<T> c = pow(a, b);
    is_about(real(c), real(x));
    is_about(imag(c), imag(x));
}

template <class T>
void
test()
{
    test(std::complex<T>(2, 3), T(2), std::complex<T>(-5, 12));
}

void test_edges()
{
    const unsigned N = sizeof(testcases) / sizeof(testcases[0]);
    for (unsigned i = 0; i < N; ++i)
    {
        for (unsigned j = 0; j < N; ++j)
        {
            std::complex<double> r = pow(testcases[i], real(testcases[j]));
            std::complex<double> z = exp(std::complex<double>(real(testcases[j])) * log(testcases[i]));
            if (std::isnan(real(r)))
                assert(std::isnan(real(z)));
            else
            {
                assert(real(r) == real(z));
            }
            if (std::isnan(imag(r)))
                assert(std::isnan(imag(z)));
            else
            {
                assert(imag(r) == imag(z));
            }
        }
    }
}

int main()
{
    test<float>();
    test<double>();
    test<long double>();
    test_edges();
}
