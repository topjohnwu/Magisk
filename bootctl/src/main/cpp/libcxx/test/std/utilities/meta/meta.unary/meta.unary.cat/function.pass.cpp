//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// function

#include <type_traits>
#include "test_macros.h"

using namespace std;

class Class {};

enum Enum1 {};
#if TEST_STD_VER >= 11
enum class Enum2 : int {};
#else
enum Enum2 {};
#endif

template <class T>
void test()
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
    static_assert(!std::is_member_object_pointer<T>::value, "");
    static_assert(!std::is_member_function_pointer<T>::value, "");
    static_assert(!std::is_enum<T>::value, "");
    static_assert(!std::is_union<T>::value, "");
    static_assert(!std::is_class<T>::value, "");
    static_assert( std::is_function<T>::value, "");
}

// Since we can't actually add the const volatile and ref qualifiers once
// later let's use a macro to do it.
#define TEST_REGULAR(...)                 \
    test<__VA_ARGS__>();                  \
    test<__VA_ARGS__ const>();            \
    test<__VA_ARGS__ volatile>();         \
    test<__VA_ARGS__ const volatile>()


#define TEST_REF_QUALIFIED(...)           \
    test<__VA_ARGS__ &>();                \
    test<__VA_ARGS__ const &>();          \
    test<__VA_ARGS__ volatile &>();       \
    test<__VA_ARGS__ const volatile &>(); \
    test<__VA_ARGS__ &&>();               \
    test<__VA_ARGS__ const &&>();         \
    test<__VA_ARGS__ volatile &&>();      \
    test<__VA_ARGS__ const volatile &&>()

struct incomplete_type;

int main()
{
    TEST_REGULAR( void () );
    TEST_REGULAR( void (int) );
    TEST_REGULAR( int (double) );
    TEST_REGULAR( int (double, char) );
    TEST_REGULAR( void (...) );
    TEST_REGULAR( void (int, ...) );
    TEST_REGULAR( int (double, ...) );
    TEST_REGULAR( int (double, char, ...) );
#if TEST_STD_VER >= 11
    TEST_REF_QUALIFIED( void () );
    TEST_REF_QUALIFIED( void (int) );
    TEST_REF_QUALIFIED( int (double) );
    TEST_REF_QUALIFIED( int (double, char) );
    TEST_REF_QUALIFIED( void (...) );
    TEST_REF_QUALIFIED( void (int, ...) );
    TEST_REF_QUALIFIED( int (double, ...) );
    TEST_REF_QUALIFIED( int (double, char, ...) );
#endif

//  LWG#2582
    static_assert(!std::is_function<incomplete_type>::value, "");
}
