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
//   asin(const complex<T>& x);

#include <complex>
#include <cassert>

#include "../cases.h"

template <class T>
void
test(const std::complex<T>& c, std::complex<T> x)
{
    assert(asin(c) == x);
}

template <class T>
void
test()
{
    test(std::complex<T>(0, 0), std::complex<T>(0, 0));
}

void test_edges()
{
    const double pi = std::atan2(+0., -0.);
    const unsigned N = sizeof(testcases) / sizeof(testcases[0]);
    for (unsigned i = 0; i < N; ++i)
    {
        std::complex<double> r = asin(testcases[i]);
        if (testcases[i].real() == 0 && testcases[i].imag() == 0)
        {
            assert(std::signbit(r.real()) == std::signbit(testcases[i].real()));
            assert(std::signbit(r.imag()) == std::signbit(testcases[i].imag()));
        }
        else if (std::isfinite(testcases[i].real()) && std::isinf(testcases[i].imag()))
        {
            assert(r.real() == 0);
            assert(std::signbit(testcases[i].real()) == std::signbit(r.real()));
            assert(std::isinf(r.imag()));
            assert(std::signbit(testcases[i].imag()) == std::signbit(r.imag()));
        }
        else if ( testcases[i].real() == 0 && std::isnan(testcases[i].imag()))
        {
            assert(r.real() == 0);
            assert(std::signbit(testcases[i].real()) == std::signbit(r.real()));
            assert(std::isnan(r.imag()));
        }
        else if (std::isfinite(testcases[i].real()) && std::isnan(testcases[i].imag()))
        {
            assert(std::isnan(r.real()));
            assert(std::isnan(r.imag()));
        }
        else if (std::isinf(testcases[i].real()) && std::isfinite(testcases[i].imag()))
        {
            if (testcases[i].real() > 0)
                is_about(r.real(),  pi/2);
            else
                is_about(r.real(), - pi/2);
            assert(std::isinf(r.imag()));
            assert(std::signbit(testcases[i].imag()) == std::signbit(r.imag()));
        }
        else if (std::isinf(testcases[i].real()) && std::isinf(testcases[i].imag()))
        {
            if (std::signbit(testcases[i].real()))
                is_about(r.real(), -pi/4);
            else
                is_about(r.real(),  pi/4);
            assert(std::isinf(r.imag()));
            assert(std::signbit(testcases[i].imag()) == std::signbit(r.imag()));
        }
        else if (std::isinf(testcases[i].real()) && std::isnan(testcases[i].imag()))
        {
            assert(std::isnan(r.real()));
            assert(std::isinf(r.imag()));
            assert(std::signbit(testcases[i].real()) != std::signbit(r.imag()));
        }
        else if (std::isnan(testcases[i].real()) && std::isfinite(testcases[i].imag()))
        {
            assert(std::isnan(r.real()));
            assert(std::isnan(r.imag()));
        }
        else if (std::isnan(testcases[i].real()) && std::isinf(testcases[i].imag()))
        {
            assert(std::isnan(r.real()));
            assert(std::isinf(r.imag()));
        }
        else if (std::isnan(testcases[i].real()) && std::isnan(testcases[i].imag()))
        {
            assert(std::isnan(r.real()));
            assert(std::isnan(r.imag()));
        }
        else
        {
            assert(std::signbit(r.real()) == std::signbit(testcases[i].real()));
            assert(std::signbit(r.imag()) == std::signbit(testcases[i].imag()));
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
