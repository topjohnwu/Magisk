//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <complex>

// template<class T>    complex<T>           proj(const complex<T>&);
//                      complex<long double> proj(long double);
//                      complex<double>      proj(double);
// template<Integral T> complex<double>      proj(T);
//                      complex<float>       proj(float);

#include <complex>
#include <type_traits>
#include <cassert>

#include "../cases.h"

template <class T>
void
test(T x, typename std::enable_if<std::is_integral<T>::value>::type* = 0)
{
    static_assert((std::is_same<decltype(std::proj(x)), std::complex<double> >::value), "");
    assert(std::proj(x) == proj(std::complex<double>(x, 0)));
}

template <class T>
void
test(T x, typename std::enable_if<std::is_floating_point<T>::value>::type* = 0)
{
    static_assert((std::is_same<decltype(std::proj(x)), std::complex<T> >::value), "");
    assert(std::proj(x) == proj(std::complex<T>(x, 0)));
}

template <class T>
void
test(T x, typename std::enable_if<!std::is_integral<T>::value &&
                                  !std::is_floating_point<T>::value>::type* = 0)
{
    static_assert((std::is_same<decltype(std::proj(x)), std::complex<T> >::value), "");
    assert(std::proj(x) == proj(std::complex<T>(x, 0)));
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
