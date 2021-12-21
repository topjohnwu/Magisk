//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <limits>

#include "test_macros.h"

/*
<limits>:
    numeric_limits
        is_specialized
        digits
        digits10
        max_digits10
        is_signed
        is_integer
        is_exact
        radix
        min_exponent
        min_exponent10
        max_exponent
        max_exponent10
        has_infinity
        has_quiet_NaN
        has_signaling_NaN
        has_denorm
        has_denorm_loss
        is_iec559
        is_bounded
        is_modulo
        traps
        tinyness_before
        round_style
*/

template <class T>
void test(const T &) {}

#define TEST_NUMERIC_LIMITS(type) \
  test(std::numeric_limits<type>::is_specialized); \
  test(std::numeric_limits<type>::digits); \
  test(std::numeric_limits<type>::digits10); \
  test(std::numeric_limits<type>::max_digits10); \
  test(std::numeric_limits<type>::is_signed); \
  test(std::numeric_limits<type>::is_integer); \
  test(std::numeric_limits<type>::is_exact); \
  test(std::numeric_limits<type>::radix); \
  test(std::numeric_limits<type>::min_exponent); \
  test(std::numeric_limits<type>::min_exponent10); \
  test(std::numeric_limits<type>::max_exponent); \
  test(std::numeric_limits<type>::max_exponent10); \
  test(std::numeric_limits<type>::has_infinity); \
  test(std::numeric_limits<type>::has_quiet_NaN); \
  test(std::numeric_limits<type>::has_signaling_NaN); \
  test(std::numeric_limits<type>::has_denorm); \
  test(std::numeric_limits<type>::has_denorm_loss); \
  test(std::numeric_limits<type>::is_iec559); \
  test(std::numeric_limits<type>::is_bounded); \
  test(std::numeric_limits<type>::is_modulo); \
  test(std::numeric_limits<type>::traps); \
  test(std::numeric_limits<type>::tinyness_before); \
  test(std::numeric_limits<type>::round_style);

struct other {};

int main()
{
    // bool
    TEST_NUMERIC_LIMITS(bool)
    TEST_NUMERIC_LIMITS(const bool)
    TEST_NUMERIC_LIMITS(volatile bool)
    TEST_NUMERIC_LIMITS(const volatile bool)

    // char
    TEST_NUMERIC_LIMITS(char)
    TEST_NUMERIC_LIMITS(const char)
    TEST_NUMERIC_LIMITS(volatile char)
    TEST_NUMERIC_LIMITS(const volatile char)

    // signed char
    TEST_NUMERIC_LIMITS(signed char)
    TEST_NUMERIC_LIMITS(const signed char)
    TEST_NUMERIC_LIMITS(volatile signed char)
    TEST_NUMERIC_LIMITS(const volatile signed char)

    // unsigned char
    TEST_NUMERIC_LIMITS(unsigned char)
    TEST_NUMERIC_LIMITS(const unsigned char)
    TEST_NUMERIC_LIMITS(volatile unsigned char)
    TEST_NUMERIC_LIMITS(const volatile unsigned char)

    // wchar_t
    TEST_NUMERIC_LIMITS(wchar_t)
    TEST_NUMERIC_LIMITS(const wchar_t)
    TEST_NUMERIC_LIMITS(volatile wchar_t)
    TEST_NUMERIC_LIMITS(const volatile wchar_t)

#if TEST_STD_VER > 17 && defined(__cpp_char8_t)
    // char8_t
    TEST_NUMERIC_LIMITS(char8_t)
    TEST_NUMERIC_LIMITS(const char8_t)
    TEST_NUMERIC_LIMITS(volatile char8_t)
    TEST_NUMERIC_LIMITS(const volatile char8_t)
#endif

    // char16_t
    TEST_NUMERIC_LIMITS(char16_t)
    TEST_NUMERIC_LIMITS(const char16_t)
    TEST_NUMERIC_LIMITS(volatile char16_t)
    TEST_NUMERIC_LIMITS(const volatile char16_t)

    // char32_t
    TEST_NUMERIC_LIMITS(char32_t)
    TEST_NUMERIC_LIMITS(const char32_t)
    TEST_NUMERIC_LIMITS(volatile char32_t)
    TEST_NUMERIC_LIMITS(const volatile char32_t)

    // short
    TEST_NUMERIC_LIMITS(short)
    TEST_NUMERIC_LIMITS(const short)
    TEST_NUMERIC_LIMITS(volatile short)
    TEST_NUMERIC_LIMITS(const volatile short)

    // int
    TEST_NUMERIC_LIMITS(int)
    TEST_NUMERIC_LIMITS(const int)
    TEST_NUMERIC_LIMITS(volatile int)
    TEST_NUMERIC_LIMITS(const volatile int)

    // long
    TEST_NUMERIC_LIMITS(long)
    TEST_NUMERIC_LIMITS(const long)
    TEST_NUMERIC_LIMITS(volatile long)
    TEST_NUMERIC_LIMITS(const volatile long)

#ifndef _LIBCPP_HAS_NO_INT128
    TEST_NUMERIC_LIMITS(__int128_t)
    TEST_NUMERIC_LIMITS(const __int128_t)
    TEST_NUMERIC_LIMITS(volatile __int128_t)
    TEST_NUMERIC_LIMITS(const volatile __int128_t)
#endif

    // long long
    TEST_NUMERIC_LIMITS(long long)
    TEST_NUMERIC_LIMITS(const long long)
    TEST_NUMERIC_LIMITS(volatile long long)
    TEST_NUMERIC_LIMITS(const volatile long long)

    // unsigned short
    TEST_NUMERIC_LIMITS(unsigned short)
    TEST_NUMERIC_LIMITS(const unsigned short)
    TEST_NUMERIC_LIMITS(volatile unsigned short)
    TEST_NUMERIC_LIMITS(const volatile unsigned short)

    // unsigned int
    TEST_NUMERIC_LIMITS(unsigned int)
    TEST_NUMERIC_LIMITS(const unsigned int)
    TEST_NUMERIC_LIMITS(volatile unsigned int)
    TEST_NUMERIC_LIMITS(const volatile unsigned int)

    // unsigned long
    TEST_NUMERIC_LIMITS(unsigned long)
    TEST_NUMERIC_LIMITS(const unsigned long)
    TEST_NUMERIC_LIMITS(volatile unsigned long)
    TEST_NUMERIC_LIMITS(const volatile unsigned long)

    // unsigned long long
    TEST_NUMERIC_LIMITS(unsigned long long)
    TEST_NUMERIC_LIMITS(const unsigned long long)
    TEST_NUMERIC_LIMITS(volatile unsigned long long)
    TEST_NUMERIC_LIMITS(const volatile unsigned long long)

#ifndef _LIBCPP_HAS_NO_INT128
    TEST_NUMERIC_LIMITS(__uint128_t)
    TEST_NUMERIC_LIMITS(const __uint128_t)
    TEST_NUMERIC_LIMITS(volatile __uint128_t)
    TEST_NUMERIC_LIMITS(const volatile __uint128_t)
#endif

    // float
    TEST_NUMERIC_LIMITS(float)
    TEST_NUMERIC_LIMITS(const float)
    TEST_NUMERIC_LIMITS(volatile float)
    TEST_NUMERIC_LIMITS(const volatile float)

    // double
    TEST_NUMERIC_LIMITS(double)
    TEST_NUMERIC_LIMITS(const double)
    TEST_NUMERIC_LIMITS(volatile double)
    TEST_NUMERIC_LIMITS(const volatile double)

    // long double
    TEST_NUMERIC_LIMITS(long double)
    TEST_NUMERIC_LIMITS(const long double)
    TEST_NUMERIC_LIMITS(volatile long double)
    TEST_NUMERIC_LIMITS(const volatile long double)

    // other
    TEST_NUMERIC_LIMITS(other)
    TEST_NUMERIC_LIMITS(const other)
    TEST_NUMERIC_LIMITS(volatile other)
    TEST_NUMERIC_LIMITS(const volatile other)
}
