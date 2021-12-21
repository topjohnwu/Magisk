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
//   arg(T x);

#include <complex>
#include <type_traits>
#include <cassert>

#include "../cases.h"

template <class T>
void
test(T x, typename std::enable_if<std::is_integral<T>::value>::type* = 0)
{
    static_assert((std::is_same<decltype(std::arg(x)), double>::value), "");
    assert(std::arg(x) == arg(std::complex<double>(static_cast<double>(x), 0)));
}

template <class T>
void
test(T x, typename std::enable_if<!std::is_integral<T>::value>::type* = 0)
{
    static_assert((std::is_same<decltype(std::arg(x)), T>::value), "");
    assert(std::arg(x) == arg(std::complex<T>(x, 0)));
}

template <class T>
void
test()
{
    test<T>(0);
    test<T>(1);
    test<T>(10);
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
