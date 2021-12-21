//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <complex>

// template<Arithmetic T, Arithmetic U>
//   complex<promote<T, U>::type>
//   pow(const T& x, const complex<U>& y);

// template<Arithmetic T, Arithmetic U>
//   complex<promote<T, U>::type>
//   pow(const complex<T>& x, const U& y);

// template<Arithmetic T, Arithmetic U>
//   complex<promote<T, U>::type>
//   pow(const complex<T>& x, const complex<U>& y);

#include <complex>
#include <type_traits>
#include <cassert>

#include "../cases.h"

template <class T>
double
promote(T, typename std::enable_if<std::is_integral<T>::value>::type* = 0);

float promote(float);
double promote(double);
long double promote(long double);

template <class T, class U>
void
test(T x, const std::complex<U>& y)
{
    typedef decltype(promote(x)+promote(real(y))) V;
    static_assert((std::is_same<decltype(std::pow(x, y)), std::complex<V> >::value), "");
    assert(std::pow(x, y) == pow(std::complex<V>(x, 0), std::complex<V>(y)));
}

template <class T, class U>
void
test(const std::complex<T>& x, U y)
{
    typedef decltype(promote(real(x))+promote(y)) V;
    static_assert((std::is_same<decltype(std::pow(x, y)), std::complex<V> >::value), "");
    assert(std::pow(x, y) == pow(std::complex<V>(x), std::complex<V>(y, 0)));
}

template <class T, class U>
void
test(const std::complex<T>& x, const std::complex<U>& y)
{
    typedef decltype(promote(real(x))+promote(real(y))) V;
    static_assert((std::is_same<decltype(std::pow(x, y)), std::complex<V> >::value), "");
    assert(std::pow(x, y) == pow(std::complex<V>(x), std::complex<V>(y)));
}

template <class T, class U>
void
test(typename std::enable_if<std::is_integral<T>::value>::type* = 0, typename std::enable_if<!std::is_integral<U>::value>::type* = 0)
{
    test(T(3), std::complex<U>(4, 5));
    test(std::complex<U>(3, 4), T(5));
}

template <class T, class U>
void
test(typename std::enable_if<!std::is_integral<T>::value>::type* = 0, typename std::enable_if<!std::is_integral<U>::value>::type* = 0)
{
    test(T(3), std::complex<U>(4, 5));
    test(std::complex<T>(3, 4), U(5));
    test(std::complex<T>(3, 4), std::complex<U>(5, 6));
}

int main()
{
    test<int, float>();
    test<int, double>();
    test<int, long double>();

    test<unsigned, float>();
    test<unsigned, double>();
    test<unsigned, long double>();

    test<long long, float>();
    test<long long, double>();
    test<long long, long double>();

    test<float, double>();
    test<float, long double>();

    test<double, float>();
    test<double, long double>();

    test<long double, float>();
    test<long double, double>();
}
