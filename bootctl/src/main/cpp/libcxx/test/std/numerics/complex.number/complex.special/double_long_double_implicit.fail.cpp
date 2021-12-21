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
//     explicit constexpr complex(const complex<long double>&);
// };

#include <complex>
#include <cassert>

int main()
{
    const std::complex<long double> cd(2.5, 3.5);
    std::complex<double> cf = cd;
    assert(cf.real() == cd.real());
    assert(cf.imag() == cd.imag());
}
