//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// template <class T>
// struct hash
//     : public unary_function<T, size_t>
// {
//     size_t operator()(T val) const;
// };

#include <functional>
#include <cassert>
#include <type_traits>
#include <cstddef>
#include <limits>

#include "test_macros.h"

template <class T>
void
test()
{
    typedef std::hash<T> H;
    static_assert((std::is_same<typename H::argument_type, T>::value), "" );
    static_assert((std::is_same<typename H::result_type, std::size_t>::value), "" );
    ASSERT_NOEXCEPT(H()(T()));
    H h;

    for (int i = 0; i <= 5; ++i)
    {
        T t(static_cast<T>(i));
        const bool small = std::integral_constant<bool, sizeof(T) <= sizeof(std::size_t)>::value; // avoid compiler warnings
        if (small)
        {
            const std::size_t result = h(t);
            LIBCPP_ASSERT(result == static_cast<size_t>(t));
            ((void)result); // Prevent unused warning
        }
    }
}

int main()
{
    test<bool>();
    test<char>();
    test<signed char>();
    test<unsigned char>();
    test<char16_t>();
    test<char32_t>();
    test<wchar_t>();
    test<short>();
    test<unsigned short>();
    test<int>();
    test<unsigned int>();
    test<long>();
    test<unsigned long>();
    test<long long>();
    test<unsigned long long>();

//  LWG #2119
    test<std::ptrdiff_t>();
    test<size_t>();

    test<int8_t>();
    test<int16_t>();
    test<int32_t>();
    test<int64_t>();

    test<int_fast8_t>();
    test<int_fast16_t>();
    test<int_fast32_t>();
    test<int_fast64_t>();

    test<int_least8_t>();
    test<int_least16_t>();
    test<int_least32_t>();
    test<int_least64_t>();

    test<intmax_t>();
    test<intptr_t>();

    test<uint8_t>();
    test<uint16_t>();
    test<uint32_t>();
    test<uint64_t>();

    test<uint_fast8_t>();
    test<uint_fast16_t>();
    test<uint_fast32_t>();
    test<uint_fast64_t>();

    test<uint_least8_t>();
    test<uint_least16_t>();
    test<uint_least32_t>();
    test<uint_least64_t>();

    test<uintmax_t>();
    test<uintptr_t>();

#ifndef _LIBCPP_HAS_NO_INT128
    test<__int128_t>();
    test<__uint128_t>();
#endif
}
