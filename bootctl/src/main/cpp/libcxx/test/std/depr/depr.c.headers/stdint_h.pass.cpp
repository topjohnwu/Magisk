//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test <stdint.h>

#include <stdint.h>
#include <cstddef>
#include <csignal>
#include <cwctype>
#include <climits>
#include <type_traits>
#include <limits>
#include <cassert>

int main()
{
    // typedef int8_t
    static_assert(sizeof(int8_t)*CHAR_BIT == 8,
                 "sizeof(int8_t)*CHAR_BIT == 8");
    static_assert(std::is_signed<int8_t>::value,
                 "std::is_signed<int8_t>::value");
    // typedef int16_t
    static_assert(sizeof(int16_t)*CHAR_BIT == 16,
                 "sizeof(int16_t)*CHAR_BIT == 16");
    static_assert(std::is_signed<int16_t>::value,
                 "std::is_signed<int16_t>::value");
    // typedef int32_t
    static_assert(sizeof(int32_t)*CHAR_BIT == 32,
                 "sizeof(int32_t)*CHAR_BIT == 32");
    static_assert(std::is_signed<int32_t>::value,
                 "std::is_signed<int32_t>::value");
    // typedef int64_t
    static_assert(sizeof(int64_t)*CHAR_BIT == 64,
                 "sizeof(int64_t)*CHAR_BIT == 64");
    static_assert(std::is_signed<int64_t>::value,
                 "std::is_signed<int64_t>::value");

    // typedef uint8_t
    static_assert(sizeof(uint8_t)*CHAR_BIT == 8,
                 "sizeof(uint8_t)*CHAR_BIT == 8");
    static_assert(std::is_unsigned<uint8_t>::value,
                 "std::is_unsigned<uint8_t>::value");
    // typedef uint16_t
    static_assert(sizeof(uint16_t)*CHAR_BIT == 16,
                 "sizeof(uint16_t)*CHAR_BIT == 16");
    static_assert(std::is_unsigned<uint16_t>::value,
                 "std::is_unsigned<uint16_t>::value");
    // typedef uint32_t
    static_assert(sizeof(uint32_t)*CHAR_BIT == 32,
                 "sizeof(uint32_t)*CHAR_BIT == 32");
    static_assert(std::is_unsigned<uint32_t>::value,
                 "std::is_unsigned<uint32_t>::value");
    // typedef uint64_t
    static_assert(sizeof(uint64_t)*CHAR_BIT == 64,
                 "sizeof(uint64_t)*CHAR_BIT == 64");
    static_assert(std::is_unsigned<uint64_t>::value,
                 "std::is_unsigned<uint64_t>::value");

    // typedef int_least8_t
    static_assert(sizeof(int_least8_t)*CHAR_BIT >= 8,
                 "sizeof(int_least8_t)*CHAR_BIT >= 8");
    static_assert(std::is_signed<int_least8_t>::value,
                 "std::is_signed<int_least8_t>::value");
    // typedef int_least16_t
    static_assert(sizeof(int_least16_t)*CHAR_BIT >= 16,
                 "sizeof(int_least16_t)*CHAR_BIT >= 16");
    static_assert(std::is_signed<int_least16_t>::value,
                 "std::is_signed<int_least16_t>::value");
    // typedef int_least32_t
    static_assert(sizeof(int_least32_t)*CHAR_BIT >= 32,
                 "sizeof(int_least32_t)*CHAR_BIT >= 32");
    static_assert(std::is_signed<int_least32_t>::value,
                 "std::is_signed<int_least32_t>::value");
    // typedef int_least64_t
    static_assert(sizeof(int_least64_t)*CHAR_BIT >= 64,
                 "sizeof(int_least64_t)*CHAR_BIT >= 64");
    static_assert(std::is_signed<int_least64_t>::value,
                 "std::is_signed<int_least64_t>::value");

    // typedef uint_least8_t
    static_assert(sizeof(uint_least8_t)*CHAR_BIT >= 8,
                 "sizeof(uint_least8_t)*CHAR_BIT >= 8");
    static_assert(std::is_unsigned<uint_least8_t>::value,
                 "std::is_unsigned<uint_least8_t>::value");
    // typedef uint_least16_t
    static_assert(sizeof(uint_least16_t)*CHAR_BIT >= 16,
                 "sizeof(uint_least16_t)*CHAR_BIT >= 16");
    static_assert(std::is_unsigned<uint_least16_t>::value,
                 "std::is_unsigned<uint_least16_t>::value");
    // typedef uint_least32_t
    static_assert(sizeof(uint_least32_t)*CHAR_BIT >= 32,
                 "sizeof(uint_least32_t)*CHAR_BIT >= 32");
    static_assert(std::is_unsigned<uint_least32_t>::value,
                 "std::is_unsigned<uint_least32_t>::value");
    // typedef uint_least64_t
    static_assert(sizeof(uint_least64_t)*CHAR_BIT >= 64,
                 "sizeof(uint_least64_t)*CHAR_BIT >= 64");
    static_assert(std::is_unsigned<uint_least64_t>::value,
                 "std::is_unsigned<uint_least64_t>::value");

    // typedef int_fast8_t
    static_assert(sizeof(int_fast8_t)*CHAR_BIT >= 8,
                 "sizeof(int_fast8_t)*CHAR_BIT >= 8");
    static_assert(std::is_signed<int_fast8_t>::value,
                 "std::is_signed<int_fast8_t>::value");
    // typedef int_fast16_t
    static_assert(sizeof(int_fast16_t)*CHAR_BIT >= 16,
                 "sizeof(int_fast16_t)*CHAR_BIT >= 16");
    static_assert(std::is_signed<int_fast16_t>::value,
                 "std::is_signed<int_fast16_t>::value");
    // typedef int_fast32_t
    static_assert(sizeof(int_fast32_t)*CHAR_BIT >= 32,
                 "sizeof(int_fast32_t)*CHAR_BIT >= 32");
    static_assert(std::is_signed<int_fast32_t>::value,
                 "std::is_signed<int_fast32_t>::value");
    // typedef int_fast64_t
    static_assert(sizeof(int_fast64_t)*CHAR_BIT >= 64,
                 "sizeof(int_fast64_t)*CHAR_BIT >= 64");
    static_assert(std::is_signed<int_fast64_t>::value,
                 "std::is_signed<int_fast64_t>::value");

    // typedef uint_fast8_t
    static_assert(sizeof(uint_fast8_t)*CHAR_BIT >= 8,
                 "sizeof(uint_fast8_t)*CHAR_BIT >= 8");
    static_assert(std::is_unsigned<uint_fast8_t>::value,
                 "std::is_unsigned<uint_fast8_t>::value");
    // typedef uint_fast16_t
    static_assert(sizeof(uint_fast16_t)*CHAR_BIT >= 16,
                 "sizeof(uint_fast16_t)*CHAR_BIT >= 16");
    static_assert(std::is_unsigned<uint_fast16_t>::value,
                 "std::is_unsigned<uint_fast16_t>::value");
    // typedef uint_fast32_t
    static_assert(sizeof(uint_fast32_t)*CHAR_BIT >= 32,
                 "sizeof(uint_fast32_t)*CHAR_BIT >= 32");
    static_assert(std::is_unsigned<uint_fast32_t>::value,
                 "std::is_unsigned<uint_fast32_t>::value");
    // typedef uint_fast64_t
    static_assert(sizeof(uint_fast64_t)*CHAR_BIT >= 64,
                 "sizeof(uint_fast64_t)*CHAR_BIT >= 64");
    static_assert(std::is_unsigned<uint_fast64_t>::value,
                 "std::is_unsigned<uint_fast64_t>::value");

    // typedef intptr_t
    static_assert(sizeof(intptr_t) >= sizeof(void*),
                 "sizeof(intptr_t) >= sizeof(void*)");
    static_assert(std::is_signed<intptr_t>::value,
                 "std::is_signed<intptr_t>::value");
    // typedef uintptr_t
    static_assert(sizeof(uintptr_t) >= sizeof(void*),
                 "sizeof(uintptr_t) >= sizeof(void*)");
    static_assert(std::is_unsigned<uintptr_t>::value,
                 "std::is_unsigned<uintptr_t>::value");

    // typedef intmax_t
    static_assert(sizeof(intmax_t) >= sizeof(long long),
                 "sizeof(intmax_t) >= sizeof(long long)");
    static_assert(std::is_signed<intmax_t>::value,
                 "std::is_signed<intmax_t>::value");
    // typedef uintmax_t
    static_assert(sizeof(uintmax_t) >= sizeof(unsigned long long),
                 "sizeof(uintmax_t) >= sizeof(unsigned long long)");
    static_assert(std::is_unsigned<uintmax_t>::value,
                 "std::is_unsigned<uintmax_t>::value");

    // INTN_MIN
    static_assert(INT8_MIN == -128, "INT8_MIN == -128");
    static_assert(INT16_MIN == -32768, "INT16_MIN == -32768");
    static_assert(INT32_MIN == -2147483647 - 1, "INT32_MIN == -2147483648");
    static_assert(INT64_MIN == -9223372036854775807LL - 1, "INT64_MIN == -9223372036854775808LL");

    // INTN_MAX
    static_assert(INT8_MAX == 127, "INT8_MAX == 127");
    static_assert(INT16_MAX == 32767, "INT16_MAX == 32767");
    static_assert(INT32_MAX == 2147483647, "INT32_MAX == 2147483647");
    static_assert(INT64_MAX == 9223372036854775807LL, "INT64_MAX == 9223372036854775807LL");

    // UINTN_MAX
    static_assert(UINT8_MAX == 255, "UINT8_MAX == 255");
    static_assert(UINT16_MAX == 65535, "UINT16_MAX == 65535");
    static_assert(UINT32_MAX == 4294967295U, "UINT32_MAX == 4294967295");
    static_assert(UINT64_MAX == 18446744073709551615ULL, "UINT64_MAX == 18446744073709551615ULL");

    // INT_FASTN_MIN
    static_assert(INT_FAST8_MIN <= -128, "INT_FAST8_MIN <= -128");
    static_assert(INT_FAST16_MIN <= -32768, "INT_FAST16_MIN <= -32768");
    static_assert(INT_FAST32_MIN <= -2147483647 - 1, "INT_FAST32_MIN <= -2147483648");
    static_assert(INT_FAST64_MIN <= -9223372036854775807LL - 1, "INT_FAST64_MIN <= -9223372036854775808LL");

    // INT_FASTN_MAX
    static_assert(INT_FAST8_MAX >= 127, "INT_FAST8_MAX >= 127");
    static_assert(INT_FAST16_MAX >= 32767, "INT_FAST16_MAX >= 32767");
    static_assert(INT_FAST32_MAX >= 2147483647, "INT_FAST32_MAX >= 2147483647");
    static_assert(INT_FAST64_MAX >= 9223372036854775807LL, "INT_FAST64_MAX >= 9223372036854775807LL");

    // UINT_FASTN_MAX
    static_assert(UINT_FAST8_MAX >= 255, "UINT_FAST8_MAX >= 255");
    static_assert(UINT_FAST16_MAX >= 65535, "UINT_FAST16_MAX >= 65535");
    static_assert(UINT_FAST32_MAX >= 4294967295U, "UINT_FAST32_MAX >= 4294967295");
    static_assert(UINT_FAST64_MAX >= 18446744073709551615ULL, "UINT_FAST64_MAX >= 18446744073709551615ULL");

    // INTPTR_MIN
    assert(INTPTR_MIN == std::numeric_limits<intptr_t>::min());

    // INTPTR_MAX
    assert(INTPTR_MAX == std::numeric_limits<intptr_t>::max());

    // UINTPTR_MAX
    assert(UINTPTR_MAX == std::numeric_limits<uintptr_t>::max());

    // INTMAX_MIN
    assert(INTMAX_MIN == std::numeric_limits<intmax_t>::min());

    // INTMAX_MAX
    assert(INTMAX_MAX == std::numeric_limits<intmax_t>::max());

    // UINTMAX_MAX
    assert(UINTMAX_MAX == std::numeric_limits<uintmax_t>::max());

    // PTRDIFF_MIN
    assert(PTRDIFF_MIN == std::numeric_limits<std::ptrdiff_t>::min());

    // PTRDIFF_MAX
    assert(PTRDIFF_MAX == std::numeric_limits<std::ptrdiff_t>::max());

    // SIG_ATOMIC_MIN
    assert(SIG_ATOMIC_MIN == std::numeric_limits<sig_atomic_t>::min());

    // SIG_ATOMIC_MAX
    assert(SIG_ATOMIC_MAX == std::numeric_limits<sig_atomic_t>::max());

    // SIZE_MAX
    assert(SIZE_MAX == std::numeric_limits<size_t>::max());

    // WCHAR_MIN
    assert(WCHAR_MIN == std::numeric_limits<wchar_t>::min());

    // WCHAR_MAX
    assert(WCHAR_MAX == std::numeric_limits<wchar_t>::max());

    // WINT_MIN
    assert(WINT_MIN == std::numeric_limits<wint_t>::min());

    // WINT_MAX
    assert(WINT_MAX == std::numeric_limits<wint_t>::max());

#ifndef INT8_C
#error INT8_C not defined
#endif

#ifndef INT16_C
#error INT16_C not defined
#endif

#ifndef INT32_C
#error INT32_C not defined
#endif

#ifndef INT64_C
#error INT64_C not defined
#endif

#ifndef UINT8_C
#error UINT8_C not defined
#endif

#ifndef UINT16_C
#error UINT16_C not defined
#endif

#ifndef UINT32_C
#error UINT32_C not defined
#endif

#ifndef UINT64_C
#error UINT64_C not defined
#endif

#ifndef INTMAX_C
#error INTMAX_C not defined
#endif

#ifndef UINTMAX_C
#error UINTMAX_C not defined
#endif
}
