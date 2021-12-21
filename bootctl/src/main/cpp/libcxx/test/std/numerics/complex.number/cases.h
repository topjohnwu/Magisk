//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <complex>

// test cases

#ifndef CASES_H
#define CASES_H

#include <complex>
#include <cassert>

const std::complex<double> testcases[] =
{
    std::complex<double>( 1.e-6,  1.e-6),
    std::complex<double>(-1.e-6,  1.e-6),
    std::complex<double>(-1.e-6, -1.e-6),
    std::complex<double>( 1.e-6, -1.e-6),

    std::complex<double>( 1.e+6,  1.e-6),
    std::complex<double>(-1.e+6,  1.e-6),
    std::complex<double>(-1.e+6, -1.e-6),
    std::complex<double>( 1.e+6, -1.e-6),

    std::complex<double>( 1.e-6,  1.e+6),
    std::complex<double>(-1.e-6,  1.e+6),
    std::complex<double>(-1.e-6, -1.e+6),
    std::complex<double>( 1.e-6, -1.e+6),

    std::complex<double>( 1.e+6,  1.e+6),
    std::complex<double>(-1.e+6,  1.e+6),
    std::complex<double>(-1.e+6, -1.e+6),
    std::complex<double>( 1.e+6, -1.e+6),

    std::complex<double>(NAN, NAN),
    std::complex<double>(-INFINITY, NAN),
    std::complex<double>(-2, NAN),
    std::complex<double>(-1, NAN),
    std::complex<double>(-0.5, NAN),
    std::complex<double>(-0., NAN),
    std::complex<double>(+0., NAN),
    std::complex<double>(0.5, NAN),
    std::complex<double>(1, NAN),
    std::complex<double>(2, NAN),
    std::complex<double>(INFINITY, NAN),

    std::complex<double>(NAN, -INFINITY),
    std::complex<double>(-INFINITY, -INFINITY),
    std::complex<double>(-2, -INFINITY),
    std::complex<double>(-1, -INFINITY),
    std::complex<double>(-0.5, -INFINITY),
    std::complex<double>(-0., -INFINITY),
    std::complex<double>(+0., -INFINITY),
    std::complex<double>(0.5, -INFINITY),
    std::complex<double>(1, -INFINITY),
    std::complex<double>(2, -INFINITY),
    std::complex<double>(INFINITY, -INFINITY),

    std::complex<double>(NAN, -2),
    std::complex<double>(-INFINITY, -2),
    std::complex<double>(-2, -2),
    std::complex<double>(-1, -2),
    std::complex<double>(-0.5, -2),
    std::complex<double>(-0., -2),
    std::complex<double>(+0., -2),
    std::complex<double>(0.5, -2),
    std::complex<double>(1, -2),
    std::complex<double>(2, -2),
    std::complex<double>(INFINITY, -2),

    std::complex<double>(NAN, -1),
    std::complex<double>(-INFINITY, -1),
    std::complex<double>(-2, -1),
    std::complex<double>(-1, -1),
    std::complex<double>(-0.5, -1),
    std::complex<double>(-0., -1),
    std::complex<double>(+0., -1),
    std::complex<double>(0.5, -1),
    std::complex<double>(1, -1),
    std::complex<double>(2, -1),
    std::complex<double>(INFINITY, -1),

    std::complex<double>(NAN, -0.5),
    std::complex<double>(-INFINITY, -0.5),
    std::complex<double>(-2, -0.5),
    std::complex<double>(-1, -0.5),
    std::complex<double>(-0.5, -0.5),
    std::complex<double>(-0., -0.5),
    std::complex<double>(+0., -0.5),
    std::complex<double>(0.5, -0.5),
    std::complex<double>(1, -0.5),
    std::complex<double>(2, -0.5),
    std::complex<double>(INFINITY, -0.5),

    std::complex<double>(NAN, -0.),
    std::complex<double>(-INFINITY, -0.),
    std::complex<double>(-2, -0.),
    std::complex<double>(-1, -0.),
    std::complex<double>(-0.5, -0.),
    std::complex<double>(-0., -0.),
    std::complex<double>(+0., -0.),
    std::complex<double>(0.5, -0.),
    std::complex<double>(1, -0.),
    std::complex<double>(2, -0.),
    std::complex<double>(INFINITY, -0.),

    std::complex<double>(NAN, +0.),
    std::complex<double>(-INFINITY, +0.),
    std::complex<double>(-2, +0.),
    std::complex<double>(-1, +0.),
    std::complex<double>(-0.5, +0.),
    std::complex<double>(-0., +0.),
    std::complex<double>(+0., +0.),
    std::complex<double>(0.5, +0.),
    std::complex<double>(1, +0.),
    std::complex<double>(2, +0.),
    std::complex<double>(INFINITY, +0.),

    std::complex<double>(NAN, 0.5),
    std::complex<double>(-INFINITY, 0.5),
    std::complex<double>(-2, 0.5),
    std::complex<double>(-1, 0.5),
    std::complex<double>(-0.5, 0.5),
    std::complex<double>(-0., 0.5),
    std::complex<double>(+0., 0.5),
    std::complex<double>(0.5, 0.5),
    std::complex<double>(1, 0.5),
    std::complex<double>(2, 0.5),
    std::complex<double>(INFINITY, 0.5),

    std::complex<double>(NAN, 1),
    std::complex<double>(-INFINITY, 1),
    std::complex<double>(-2, 1),
    std::complex<double>(-1, 1),
    std::complex<double>(-0.5, 1),
    std::complex<double>(-0., 1),
    std::complex<double>(+0., 1),
    std::complex<double>(0.5, 1),
    std::complex<double>(1, 1),
    std::complex<double>(2, 1),
    std::complex<double>(INFINITY, 1),

    std::complex<double>(NAN, 2),
    std::complex<double>(-INFINITY, 2),
    std::complex<double>(-2, 2),
    std::complex<double>(-1, 2),
    std::complex<double>(-0.5, 2),
    std::complex<double>(-0., 2),
    std::complex<double>(+0., 2),
    std::complex<double>(0.5, 2),
    std::complex<double>(1, 2),
    std::complex<double>(2, 2),
    std::complex<double>(INFINITY, 2),

    std::complex<double>(NAN, INFINITY),
    std::complex<double>(-INFINITY, INFINITY),
    std::complex<double>(-2, INFINITY),
    std::complex<double>(-1, INFINITY),
    std::complex<double>(-0.5, INFINITY),
    std::complex<double>(-0., INFINITY),
    std::complex<double>(+0., INFINITY),
    std::complex<double>(0.5, INFINITY),
    std::complex<double>(1, INFINITY),
    std::complex<double>(2, INFINITY),
    std::complex<double>(INFINITY, INFINITY)
};

enum {zero, non_zero, inf, NaN, non_zero_nan};

template <class T>
int
classify(const std::complex<T>& x)
{
    if (x == std::complex<T>())
        return zero;
    if (std::isinf(x.real()) || std::isinf(x.imag()))
        return inf;
    if (std::isnan(x.real()) && std::isnan(x.imag()))
        return NaN;
    if (std::isnan(x.real()))
    {
        if (x.imag() == T(0))
            return NaN;
        return non_zero_nan;
    }
    if (std::isnan(x.imag()))
    {
        if (x.real() == T(0))
            return NaN;
        return non_zero_nan;
    }
    return non_zero;
}

inline
int
classify(double x)
{
    if (x == 0)
        return zero;
    if (std::isinf(x))
        return inf;
    if (std::isnan(x))
        return NaN;
    return non_zero;
}

void is_about(float x, float y)
{
    assert(std::abs((x-y)/(x+y)) < 1.e-6);
}

void is_about(double x, double y)
{
    assert(std::abs((x-y)/(x+y)) < 1.e-14);
}

void is_about(long double x, long double y)
{
    assert(std::abs((x-y)/(x+y)) < 1.e-14);
}

#endif  // CASES_H
