//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11
// <chrono>

#include <complex>
#include <type_traits>
#include <cassert>

int main()
{
    using namespace std;

    {
    std::complex<long double> c1 = 3.0il;
    assert ( c1 == std::complex<long double>(0, 3.0));
    auto c2 = 3il;
    assert ( c1 == c2 );
    }

    {
    std::complex<double> c1 = 3.0i;
    assert ( c1 == std::complex<double>(0, 3.0));
    auto c2 = 3i;
    assert ( c1 == c2 );
    }

    {
    std::complex<float> c1 = 3.0if;
    assert ( c1 == std::complex<float>(0, 3.0));
    auto c2 = 3if;
    assert ( c1 == c2 );
    }
}
