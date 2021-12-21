//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_integral

#include <type_traits>
#include <cstddef>        // for std::nullptr_t
#include "test_macros.h"

template <class T>
void test_is_integral()
{
    static_assert( std::is_integral<T>::value, "");
    static_assert( std::is_integral<const T>::value, "");
    static_assert( std::is_integral<volatile T>::value, "");
    static_assert( std::is_integral<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_integral_v<T>, "");
    static_assert( std::is_integral_v<const T>, "");
    static_assert( std::is_integral_v<volatile T>, "");
    static_assert( std::is_integral_v<const volatile T>, "");
#endif
}

template <class T>
void test_is_not_integral()
{
    static_assert(!std::is_integral<T>::value, "");
    static_assert(!std::is_integral<const T>::value, "");
    static_assert(!std::is_integral<volatile T>::value, "");
    static_assert(!std::is_integral<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_integral_v<T>, "");
    static_assert(!std::is_integral_v<const T>, "");
    static_assert(!std::is_integral_v<volatile T>, "");
    static_assert(!std::is_integral_v<const volatile T>, "");
#endif
}

class Empty
{
};

class NotEmpty
{
    virtual ~NotEmpty();
};

union Union {};

struct bit_zero
{
    int :  0;
};

class Abstract
{
    virtual ~Abstract() = 0;
};

enum Enum {zero, one};
struct incomplete_type;

typedef void (*FunctionPtr)();


int main()
{
    test_is_integral<short>();
    test_is_integral<unsigned short>();
    test_is_integral<int>();
    test_is_integral<unsigned int>();
    test_is_integral<long>();
    test_is_integral<unsigned long>();
    test_is_integral<bool>();
    test_is_integral<char>();
    test_is_integral<signed char>();
    test_is_integral<unsigned char>();
    test_is_integral<wchar_t>();
#if defined(__cpp_lib_char8_t) && __cpp_lib_char8_t >= 201811L
    test_is_integral<char8_t>();
#endif

    test_is_not_integral<std::nullptr_t>();
    test_is_not_integral<void>();
    test_is_not_integral<int&>();
    test_is_not_integral<int&&>();
    test_is_not_integral<int*>();
    test_is_not_integral<double>();
    test_is_not_integral<const int*>();
    test_is_not_integral<char[3]>();
    test_is_not_integral<char[]>();
    test_is_not_integral<Union>();
    test_is_not_integral<Enum>();
    test_is_not_integral<FunctionPtr>();
    test_is_not_integral<Empty>();
    test_is_not_integral<bit_zero>();
    test_is_not_integral<NotEmpty>();
    test_is_not_integral<Abstract>();
    test_is_not_integral<incomplete_type>();
}
