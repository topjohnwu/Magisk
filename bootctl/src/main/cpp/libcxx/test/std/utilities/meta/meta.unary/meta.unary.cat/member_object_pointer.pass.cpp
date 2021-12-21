//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// member_object_pointer

#include <type_traits>
#include "test_macros.h"

template <class T>
void test_member_object_pointer_imp()
{
    static_assert(!std::is_void<T>::value, "");
#if TEST_STD_VER > 11
    static_assert(!std::is_null_pointer<T>::value, "");
#endif
    static_assert(!std::is_integral<T>::value, "");
    static_assert(!std::is_floating_point<T>::value, "");
    static_assert(!std::is_array<T>::value, "");
    static_assert(!std::is_pointer<T>::value, "");
    static_assert(!std::is_lvalue_reference<T>::value, "");
    static_assert(!std::is_rvalue_reference<T>::value, "");
    static_assert( std::is_member_object_pointer<T>::value, "");
    static_assert(!std::is_member_function_pointer<T>::value, "");
    static_assert(!std::is_enum<T>::value, "");
    static_assert(!std::is_union<T>::value, "");
    static_assert(!std::is_class<T>::value, "");
    static_assert(!std::is_function<T>::value, "");
}

template <class T>
void test_member_object_pointer()
{
    test_member_object_pointer_imp<T>();
    test_member_object_pointer_imp<const T>();
    test_member_object_pointer_imp<volatile T>();
    test_member_object_pointer_imp<const volatile T>();
}

class Class
{
};

struct incomplete_type;

int main()
{
    test_member_object_pointer<int Class::*>();

//  LWG#2582
    static_assert(!std::is_member_object_pointer<incomplete_type>::value, "");
}
