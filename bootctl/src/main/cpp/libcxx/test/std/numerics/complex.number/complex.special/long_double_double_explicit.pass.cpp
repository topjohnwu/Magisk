//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <complex>

// template<> class complex<long double>
// {
// public:
//     constexpr complex(const complex<double>&);
// };

#include <complex>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
    const std::complex<double> cd(2.5, 3.5);
    std::complex<long double> cf(cd);
    assert(cf.real() == cd.real());
    assert(cf.imag() == cd.imag());
    }
#if TEST_STD_VER >= 11
    {
    constexpr std::complex<double> cd(2.5, 3.5);
    constexpr std::complex<long double> cf(cd);
    static_assert(cf.real() == cd.real(), "");
    static_assert(cf.imag() == cd.imag(), "");
    }
#endif
}
