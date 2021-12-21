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
//   bool
//   operator==(const complex<T>& lhs, const T& rhs);

#include <complex>
#include <cassert>

#include "test_macros.h"

template <class T>
void
test_constexpr()
{
#if TEST_STD_VER > 11
    {
    constexpr std::complex<T> lhs(1.5, 2.5);
    constexpr T rhs(-2.5);
    static_assert(!(lhs == rhs), "");
    }
    {
    constexpr std::complex<T> lhs(1.5, 0);
    constexpr T rhs(-2.5);
    static_assert(!(lhs == rhs), "");
    }
    {
    constexpr std::complex<T> lhs(1.5, 2.5);
    constexpr T rhs(1.5);
    static_assert(!(lhs == rhs), "");
    }
    {
    constexpr std::complex<T> lhs(1.5, 0);
    constexpr T rhs(1.5);
    static_assert( (lhs == rhs), "");
    }
#endif
}

template <class T>
void
test()
{
    {
    std::complex<T> lhs(1.5,  2.5);
    T rhs(-2.5);
    assert(!(lhs == rhs));
    }
    {
    std::complex<T> lhs(1.5, 0);
    T rhs(-2.5);
    assert(!(lhs == rhs));
    }
    {
    std::complex<T> lhs(1.5, 2.5);
    T rhs(1.5);
    assert(!(lhs == rhs));
    }
    {
    std::complex<T> lhs(1.5, 0);
    T rhs(1.5);
    assert( (lhs == rhs));
    }

    test_constexpr<T> ();
    }

int main()
{
    test<float>();
    test<double>();
    test<long double>();
//     test_constexpr<int> ();
}
