//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// nullptr_t
//  is_null_pointer

#include <type_traits>
#include <cstddef>        // for std::nullptr_t
#include "test_macros.h"

#if TEST_STD_VER > 11
template <class T>
void test_nullptr_imp()
{
    static_assert(!std::is_void<T>::value, "");
    static_assert( std::is_null_pointer<T>::value, "");
    static_assert(!std::is_integral<T>::value, "");
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
void test_nullptr()
{
    test_nullptr_imp<T>();
    test_nullptr_imp<const T>();
    test_nullptr_imp<volatile T>();
    test_nullptr_imp<const volatile T>();
}

struct incomplete_type;

int main()
{
    test_nullptr<std::nullptr_t>();

//  LWG#2582
    static_assert(!std::is_null_pointer<incomplete_type>::value, "");
}
#else
int main() {}
#endif
