//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <complex>

// template<Arithmetic T>
//   T
//   real(const T& x);

#include <complex>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "../cases.h"

template <class T, int x>
void
test(typename std::enable_if<std::is_integral<T>::value>::type* = 0)
{
    static_assert((std::is_same<decltype(std::real(T(x))), double>::value), "");
    assert(std::real(x) == x);
#if TEST_STD_VER > 11
    constexpr T val {x};
    static_assert(std::real(val) == val, "");
    constexpr std::complex<T> t{val, val};
    static_assert(t.real() == x, "" );
#endif
}

template <class T, int x>
void
test(typename std::enable_if<!std::is_integral<T>::value>::type* = 0)
{
    static_assert((std::is_same<decltype(std::real(T(x))), T>::value), "");
    assert(std::real(x) == x);
#if TEST_STD_VER > 11
    constexpr T val {x};
    static_assert(std::real(val) == val, "");
    constexpr std::complex<T> t{val, val};
    static_assert(t.real() == x, "" );
#endif
}

template <class T>
void
test()
{
    test<T, 0>();
    test<T, 1>();
    test<T, 10>();
}

int main()
{
    test<float>();
    test<double>();
    test<long double>();
    test<int>();
    test<unsigned>();
    test<long long>();
}
