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
//   polar(const T& rho, const T& theta = T());  // changed from '0' by LWG#2870

#include <complex>
#include <cassert>

#include "../cases.h"

template <class T>
void
test(const T& rho, std::complex<T> x)
{
    assert(std::polar(rho) == x);
}

template <class T>
void
test(const T& rho, const T& theta, std::complex<T> x)
{
    assert(std::polar(rho, theta) == x);
}

template <class T>
void
test()
{
    test(T(0), std::complex<T>(0, 0));
    test(T(1), std::complex<T>(1, 0));
    test(T(100), std::complex<T>(100, 0));
    test(T(0), T(0), std::complex<T>(0, 0));
    test(T(1), T(0), std::complex<T>(1, 0));
    test(T(100), T(0), std::complex<T>(100, 0));
}

void test_edges()
{
    const unsigned N = sizeof(testcases) / sizeof(testcases[0]);
    for (unsigned i = 0; i < N; ++i)
    {
        double r = real(testcases[i]);
        double theta = imag(testcases[i]);
        std::complex<double> z = std::polar(r, theta);
        switch (classify(r))
        {
        case zero:
            if (std::signbit(r) || classify(theta) == inf || classify(theta) == NaN)
            {
                int c = classify(z);
                assert(c == NaN || c == non_zero_nan);
            }
            else
            {
                assert(z == std::complex<double>());
            }
            break;
        case non_zero:
            if (std::signbit(r) || classify(theta) == inf || classify(theta) == NaN)
            {
                int c = classify(z);
                assert(c == NaN || c == non_zero_nan);
            }
            else
            {
                is_about(std::abs(z), r);
            }
            break;
        case inf:
            if (r < 0)
            {
                int c = classify(z);
                assert(c == NaN || c == non_zero_nan);
            }
            else
            {
                assert(classify(z) == inf);
                if (classify(theta) != NaN && classify(theta) != inf)
                {
                    assert(classify(real(z)) != NaN);
                    assert(classify(imag(z)) != NaN);
                }
            }
            break;
        case NaN:
        case non_zero_nan:
            {
                int c = classify(z);
                assert(c == NaN || c == non_zero_nan);
            }
            break;
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
