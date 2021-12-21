//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_convertible

#include <type_traits>
#include "test_macros.h"

template <class T, class U>
void test_is_convertible()
{
    static_assert((std::is_convertible<T, U>::value), "");
    static_assert((std::is_convertible<const T, U>::value), "");
    static_assert((std::is_convertible<T, const U>::value), "");
    static_assert((std::is_convertible<const T, const U>::value), "");
#if TEST_STD_VER > 14
    static_assert((std::is_convertible_v<T, U>), "");
    static_assert((std::is_convertible_v<const T, U>), "");
    static_assert((std::is_convertible_v<T, const U>), "");
    static_assert((std::is_convertible_v<const T, const U>), "");
#endif
}

template <class T, class U>
void test_is_not_convertible()
{
    static_assert((!std::is_convertible<T, U>::value), "");
    static_assert((!std::is_convertible<const T, U>::value), "");
    static_assert((!std::is_convertible<T, const U>::value), "");
    static_assert((!std::is_convertible<const T, const U>::value), "");
#if TEST_STD_VER > 14
    static_assert((!std::is_convertible_v<T, U>), "");
    static_assert((!std::is_convertible_v<const T, U>), "");
    static_assert((!std::is_convertible_v<T, const U>), "");
    static_assert((!std::is_convertible_v<const T, const U>), "");
#endif
}

typedef void Function();
typedef void ConstFunction() const;
typedef char Array[1];

struct StringType {
  StringType(const char*) {}
};

class NonCopyable {
  NonCopyable(NonCopyable&);
};

template <typename T>
class CannotInstantiate {
  enum { X = T::ThisExpressionWillBlowUp };
};

int main()
{
    // void
    test_is_convertible<void,void> ();
    test_is_not_convertible<void,Function> ();
    test_is_not_convertible<void,Function&> ();
    test_is_not_convertible<void,Function*> ();
    test_is_not_convertible<void,Array> ();
    test_is_not_convertible<void,Array&> ();
    test_is_not_convertible<void,char> ();
    test_is_not_convertible<void,char&> ();
    test_is_not_convertible<void,char*> ();
    test_is_not_convertible<char, void>();

    // Function
    test_is_not_convertible<Function, void> ();
    test_is_not_convertible<Function, Function> ();
    test_is_convertible<Function, Function&> ();
    test_is_convertible<Function, Function*> ();
    test_is_convertible<Function, Function*const> ();

#if TEST_STD_VER >= 11
    static_assert(( std::is_convertible<Function, Function&&>::value), "");
#endif

    test_is_not_convertible<Function, Array> ();
    test_is_not_convertible<Function, Array&> ();
    test_is_not_convertible<Function, char> ();
    test_is_not_convertible<Function, char&> ();
    test_is_not_convertible<Function, char*> ();

    // Function&
    test_is_not_convertible<Function&, void> ();
    test_is_not_convertible<Function&, Function> ();
    test_is_convertible<Function&, Function&> ();

    test_is_convertible<Function&, Function*> ();
    test_is_not_convertible<Function&, Array> ();
    test_is_not_convertible<Function&, Array&> ();
    test_is_not_convertible<Function&, char> ();
    test_is_not_convertible<Function&, char&> ();
    test_is_not_convertible<Function&, char*> ();

    // Function*
    test_is_not_convertible<Function*, void> ();
    test_is_not_convertible<Function*, Function> ();
    test_is_not_convertible<Function*, Function&> ();
    test_is_convertible<Function*, Function*> ();

    test_is_not_convertible<Function*, Array> ();
    test_is_not_convertible<Function*, Array&> ();
    test_is_not_convertible<Function*, char> ();
    test_is_not_convertible<Function*, char&> ();
    test_is_not_convertible<Function*, char*> ();

    // Non-referencable function type
    static_assert((!std::is_convertible<ConstFunction, Function>::value), "");
    static_assert((!std::is_convertible<ConstFunction, Function*>::value), "");
    static_assert((!std::is_convertible<ConstFunction, Function&>::value), "");
#if TEST_STD_VER >= 11
    static_assert((!std::is_convertible<ConstFunction, Function&&>::value), "");
#endif
    static_assert((!std::is_convertible<Function*, ConstFunction>::value), "");
    static_assert((!std::is_convertible<Function&, ConstFunction>::value), "");
    static_assert((!std::is_convertible<ConstFunction, ConstFunction>::value), "");
    static_assert((!std::is_convertible<ConstFunction, void>::value), "");

    // Array
    test_is_not_convertible<Array, void> ();
    test_is_not_convertible<Array, Function> ();
    test_is_not_convertible<Array, Function&> ();
    test_is_not_convertible<Array, Function*> ();
    test_is_not_convertible<Array, Array> ();

    static_assert((!std::is_convertible<Array, Array&>::value), "");
    static_assert(( std::is_convertible<Array, const Array&>::value), "");
    static_assert((!std::is_convertible<Array, const volatile Array&>::value), "");

    static_assert((!std::is_convertible<const Array, Array&>::value), "");
    static_assert(( std::is_convertible<const Array, const Array&>::value), "");
    static_assert((!std::is_convertible<Array, volatile Array&>::value), "");
    static_assert((!std::is_convertible<Array, const volatile Array&>::value), "");

#if TEST_STD_VER >= 11
    static_assert(( std::is_convertible<Array, Array&&>::value), "");
    static_assert(( std::is_convertible<Array, const Array&&>::value), "");
    static_assert(( std::is_convertible<Array, volatile Array&&>::value), "");
    static_assert(( std::is_convertible<Array, const volatile Array&&>::value), "");
    static_assert(( std::is_convertible<const Array, const Array&&>::value), "");
    static_assert((!std::is_convertible<Array&, Array&&>::value), "");
    static_assert((!std::is_convertible<Array&&, Array&>::value), "");
#endif

    test_is_not_convertible<Array, char> ();
    test_is_not_convertible<Array, char&> ();

    static_assert(( std::is_convertible<Array, char*>::value), "");
    static_assert(( std::is_convertible<Array, const char*>::value), "");
    static_assert(( std::is_convertible<Array, char* const>::value), "");
    static_assert(( std::is_convertible<Array, char* const volatile>::value), "");

    static_assert((!std::is_convertible<const Array, char*>::value), "");
    static_assert(( std::is_convertible<const Array, const char*>::value), "");

    static_assert((!std::is_convertible<char[42][42], char*>::value), "");
    static_assert((!std::is_convertible<char[][1], char*>::value), "");

    // Array&
    test_is_not_convertible<Array&, void> ();
    test_is_not_convertible<Array&, Function> ();
    test_is_not_convertible<Array&, Function&> ();
    test_is_not_convertible<Array&, Function*> ();
    test_is_not_convertible<Array&, Array> ();

    static_assert(( std::is_convertible<Array&, Array&>::value), "");
    static_assert(( std::is_convertible<Array&, const Array&>::value), "");
    static_assert((!std::is_convertible<const Array&, Array&>::value), "");
    static_assert(( std::is_convertible<const Array&, const Array&>::value), "");

    test_is_not_convertible<Array&, char> ();
    test_is_not_convertible<Array&, char&> ();

    static_assert(( std::is_convertible<Array&, char*>::value), "");
    static_assert(( std::is_convertible<Array&, const char*>::value), "");
    static_assert((!std::is_convertible<const Array&, char*>::value), "");
    static_assert(( std::is_convertible<const Array&, const char*>::value), "");

    static_assert((std::is_convertible<Array, StringType>::value), "");
    static_assert((std::is_convertible<char(&)[], StringType>::value), "");

    // char
    test_is_not_convertible<char, void> ();
    test_is_not_convertible<char, Function> ();
    test_is_not_convertible<char, Function&> ();
    test_is_not_convertible<char, Function*> ();
    test_is_not_convertible<char, Array> ();
    test_is_not_convertible<char, Array&> ();

    test_is_convertible<char, char> ();

    static_assert((!std::is_convertible<char, char&>::value), "");
    static_assert(( std::is_convertible<char, const char&>::value), "");
    static_assert((!std::is_convertible<const char, char&>::value), "");
    static_assert(( std::is_convertible<const char, const char&>::value), "");

    test_is_not_convertible<char, char*> ();

    // char&
    test_is_not_convertible<char&, void> ();
    test_is_not_convertible<char&, Function> ();
    test_is_not_convertible<char&, Function&> ();
    test_is_not_convertible<char&, Function*> ();
    test_is_not_convertible<char&, Array> ();
    test_is_not_convertible<char&, Array&> ();

    test_is_convertible<char&, char> ();

    static_assert(( std::is_convertible<char&, char&>::value), "");
    static_assert(( std::is_convertible<char&, const char&>::value), "");
    static_assert((!std::is_convertible<const char&, char&>::value), "");
    static_assert(( std::is_convertible<const char&, const char&>::value), "");

    test_is_not_convertible<char&, char*> ();

    // char*
    test_is_not_convertible<char*, void> ();
    test_is_not_convertible<char*, Function> ();
    test_is_not_convertible<char*, Function&> ();
    test_is_not_convertible<char*, Function*> ();
    test_is_not_convertible<char*, Array> ();
    test_is_not_convertible<char*, Array&> ();

    test_is_not_convertible<char*, char> ();
    test_is_not_convertible<char*, char&> ();

    static_assert(( std::is_convertible<char*, char*>::value), "");
    static_assert(( std::is_convertible<char*, const char*>::value), "");
    static_assert((!std::is_convertible<const char*, char*>::value), "");
    static_assert(( std::is_convertible<const char*, const char*>::value), "");

    // NonCopyable
    static_assert((std::is_convertible<NonCopyable&, NonCopyable&>::value), "");
    static_assert((std::is_convertible<NonCopyable&, const NonCopyable&>::value), "");
    static_assert((std::is_convertible<NonCopyable&, const volatile NonCopyable&>::value), "");
    static_assert((std::is_convertible<NonCopyable&, volatile NonCopyable&>::value), "");
    static_assert((std::is_convertible<const NonCopyable&, const NonCopyable&>::value), "");
    static_assert((std::is_convertible<const NonCopyable&, const volatile NonCopyable&>::value), "");
    static_assert((std::is_convertible<volatile NonCopyable&, const volatile NonCopyable&>::value), "");
    static_assert((std::is_convertible<const volatile NonCopyable&, const volatile NonCopyable&>::value), "");
    static_assert((!std::is_convertible<const NonCopyable&, NonCopyable&>::value), "");
// This test requires Access control SFINAE which we only have in C++11 or when
// we are using the compiler builtin for is_convertible.
#if TEST_STD_VER >= 11 || !defined(_LIBCPP_USE_IS_CONVERTIBLE_FALLBACK)
    test_is_not_convertible<NonCopyable&, NonCopyable>();
#endif

    // Ensure that CannotInstantiate is not instantiated by is_convertible when it is not needed.
    // For example CannotInstantiate is instatiated as a part of ADL lookup for arguments of type CannotInstantiate*.
    static_assert((std::is_convertible<CannotInstantiate<int>*, CannotInstantiate<int>*>::value), "");
}
