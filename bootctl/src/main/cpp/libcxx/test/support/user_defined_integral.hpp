//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef SUPPORT_USER_DEFINED_INTEGRAL_HPP
#define SUPPORT_USER_DEFINED_INTEGRAL_HPP

template <class T>
struct UserDefinedIntegral
{
    UserDefinedIntegral() : value(0) {}
    UserDefinedIntegral(T v) : value(v) {}
    operator T() const { return value; }
    T value;
};

// Poison the arithmetic and comparison operations
template <class T, class U>
void operator+(UserDefinedIntegral<T>, UserDefinedIntegral<U>);

template <class T, class U>
void operator-(UserDefinedIntegral<T>, UserDefinedIntegral<U>);

template <class T, class U>
void operator*(UserDefinedIntegral<T>, UserDefinedIntegral<U>);

template <class T, class U>
void operator/(UserDefinedIntegral<T>, UserDefinedIntegral<U>);

template <class T, class U>
void operator==(UserDefinedIntegral<T>, UserDefinedIntegral<U>);

template <class T, class U>
void operator!=(UserDefinedIntegral<T>, UserDefinedIntegral<U>);

template <class T, class U>
void operator<(UserDefinedIntegral<T>, UserDefinedIntegral<U>);

template <class T, class U>
void operator>(UserDefinedIntegral<T>, UserDefinedIntegral<U>);

template <class T, class U>
void operator<=(UserDefinedIntegral<T>, UserDefinedIntegral<U>);

template <class T, class U>
void operator>=(UserDefinedIntegral<T>, UserDefinedIntegral<U>);

#endif // SUPPORT_USER_DEFINED_INTEGRAL_HPP
