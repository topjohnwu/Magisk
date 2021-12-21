//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <complex>

// template<> class complex<double>
// {
// public:
//     constexpr complex(const complex<float>&);
// };

#include <complex>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
    const std::complex<float> cd(2.5, 3.5);
    std::complex<double> cf(cd);
    assert(cf.real() == cd.real());
    assert(cf.imag() == cd.imag());
    }
#if TEST_STD_VER >= 11
    {
    constexpr std::complex<float> cd(2.5, 3.5);
    constexpr std::complex<double> cf(cd);
    static_assert(cf.real() == cd.real(), "");
    static_assert(cf.imag() == cd.imag(), "");
    }
#endif
}
