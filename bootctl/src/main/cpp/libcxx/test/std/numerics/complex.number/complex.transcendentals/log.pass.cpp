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
//   log(const complex<T>& x);

#include <complex>
#include <cassert>

#include "../cases.h"

template <class T>
void
test(const std::complex<T>& c, std::complex<T> x)
{
    assert(log(c) == x);
}

template <class T>
void
test()
{
    test(std::complex<T>(0, 0), std::complex<T>(-INFINITY, 0));
}

void test_edges()
{
    const double pi = std::atan2(+0., -0.);
    const unsigned N = sizeof(testcases) / sizeof(testcases[0]);
    for (unsigned i = 0; i < N; ++i)
    {
        std::complex<double> r = log(testcases[i]);
        if (testcases[i].real() == 0 && testcases[i].imag() == 0)
        {
            if (std::signbit(testcases[i].real()))
            {
                assert(std::isinf(r.real()));
                assert(r.real() < 0);
                if (std::signbit(testcases[i].imag()))
                    is_about(r.imag(), -pi);
                else
                    is_about(r.imag(), pi);
            }
            else
            {
                assert(std::isinf(r.real()));
                assert(r.real() < 0);
                assert(r.imag() == 0);
                assert(std::signbit(testcases[i].imag()) == std::signbit(r.imag()));
            }
        }
        else if (std::isfinite(testcases[i].real()) && std::isinf(testcases[i].imag()))
        {
            assert(std::isinf(r.real()));
            assert(r.real() > 0);
            if (testcases[i].imag() > 0)
                is_about(r.imag(), pi/2);
            else
                is_about(r.imag(), -pi/2);
        }
        else if (std::isfinite(testcases[i].real()) && std::isnan(testcases[i].imag()))
        {
            assert(std::isnan(r.real()));
            assert(std::isnan(r.imag()));
        }
        else if (std::isinf(testcases[i].real()) && testcases[i].real() < 0 && std::isfinite(testcases[i].imag()))
        {
            assert(std::isinf(r.real()) && r.real() > 0);
            if (r.imag() > 0)
                is_about(r.imag(), pi);
            else
                is_about(r.imag(), -pi);
        }
        else if (std::isinf(testcases[i].real()) && testcases[i].real() > 0 && std::isfinite(testcases[i].imag()))
        {
            assert(std::isinf(r.real()) && r.real() > 0);
            assert(r.imag() == 0);
            assert(std::signbit(testcases[i].imag()) == std::signbit(r.imag()));
        }
        else if (testcases[i].real() == 1 && testcases[i].imag() == 0)
        {
            assert(r.real() == 0);
            assert(std::signbit(r.imag()) == std::signbit(testcases[i].imag()));
        }
        else if (testcases[i].real() == 0 && testcases[i].imag() == 1)
        {
            assert(r.real() == 0);
            is_about(r.imag(), pi/2);
        }
        else if (testcases[i].real() == -1 && testcases[i].imag() == 0)
        {
            assert(r.real() == 0);
            if (std::signbit(testcases[i].imag()))
                is_about(r.imag(), -pi);
            else
                is_about(r.imag(),  pi);
        }
        else if (testcases[i].real() == 0 && testcases[i].imag() == -1)
        {
            assert(r.real() == 0);
            is_about(r.imag(), -pi/2);
        }
        else if (std::isfinite(testcases[i].real()) && std::isfinite(testcases[i].imag()) && abs(testcases[i]) < 1)
        {
            assert( std::signbit(r.real()));
            assert(std::signbit(r.imag()) == std::signbit(testcases[i].imag()));
        }
        else if (std::isfinite(testcases[i].real()) && std::isfinite(testcases[i].imag()) && abs(testcases[i]) > 1)
        {
            assert(!std::signbit(r.real()));
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
