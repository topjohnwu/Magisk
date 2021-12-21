//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// integral

#include <type_traits>
#include "test_macros.h"

template <class T>
void test_integral_imp()
{
    static_assert(!std::is_void<T>::value, "");
#if TEST_STD_VER > 11
    static_assert(!std::is_null_pointer<T>::value, "");
#endif
    static_assert( std::is_integral<T>::value, "");
    static_assert(!std::is_floating_point<T>::value, "");
    static_assert(!std::is_array<T>::value, "");
    static_assert(!std::is_pointer<T>::value, "");
    static_assert(!std::is_lvalue_reference<T>::value, "");
    static_assert(!std::is_rvalue_reference<T>::value, "");
    static_assert(!std::is_member_object_pointer<T>::value, "");
    static_assert(!std::is_member_function_pointer<T>::value, "");
    static_assert(!std::is_enum<T>::value, "");
    static_assert(!std::is_union<T>::value, "");
    static_assert(!std::is_class<T>::value, "");
    static_assert(!std::is_function<T>::value, "");
}

template <class T>
void test_integral()
{
    test_integral_imp<T>();
    test_integral_imp<const T>();
    test_integral_imp<volatile T>();
    test_integral_imp<const volatile T>();
}

struct incomplete_type;

int main()
{
    test_integral<bool>();
    test_integral<char>();
    test_integral<signed char>();
    test_integral<unsigned char>();
    test_integral<wchar_t>();
    test_integral<short>();
    test_integral<unsigned short>();
    test_integral<int>();
    test_integral<unsigned int>();
    test_integral<long>();
    test_integral<unsigned long>();
    test_integral<long long>();
    test_integral<unsigned long long>();
#ifndef _LIBCPP_HAS_NO_INT128
    test_integral<__int128_t>();
    test_integral<__uint128_t>();
#endif

//  LWG#2582
    static_assert(!std::is_integral<incomplete_type>::value, "");
}
