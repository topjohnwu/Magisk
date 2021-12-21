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
//   operator!=(const T& lhs, const complex<T>& rhs);

#include <complex>
#include <cassert>

#include "test_macros.h"

template <class T>
void
test_constexpr()
{
#if TEST_STD_VER > 11
    {
    constexpr T lhs(-2.5);
    constexpr std::complex<T> rhs(1.5,  2.5);
    static_assert (lhs != rhs, "");
    }
    {
    constexpr T lhs(-2.5);
    constexpr std::complex<T> rhs(1.5,  0);
    static_assert (lhs != rhs, "");
    }
    {
    constexpr T lhs(1.5);
    constexpr std::complex<T> rhs(1.5, 2.5);
    static_assert (lhs != rhs, "");
    }
    {
    constexpr T lhs(1.5);
    constexpr std::complex<T> rhs(1.5, 0);
    static_assert (!(lhs != rhs), "");
    }
#endif
}

template <class T>
void
test()
{
    {
    T lhs(-2.5);
    std::complex<T> rhs(1.5,  2.5);
    assert (lhs != rhs);
    }
    {
    T lhs(-2.5);
    std::complex<T> rhs(1.5,  0);
    assert (lhs != rhs);
    }
    {
    T lhs(1.5);
    std::complex<T> rhs(1.5, 2.5);
    assert (lhs != rhs);
    }
    {
    T lhs(1.5);
    std::complex<T> rhs(1.5, 0);
    assert (!(lhs != rhs));
    }

    test_constexpr<T> ();
    }

int main()
{
    test<float>();
    test<double>();
    test<long double>();
//     test_constexpr<int>();
}
