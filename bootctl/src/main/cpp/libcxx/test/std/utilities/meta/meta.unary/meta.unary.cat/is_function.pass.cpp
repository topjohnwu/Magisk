//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// is_function

#include <type_traits>
#include <cstddef>        // for std::nullptr_t

#include "test_macros.h"

// NOTE: On Windows the function `test_is_function<void()>` and
// `test_is_function<void() noexcept> has the same mangled despite being
// a distinct instantiation. This causes Clang to emit an error. However
// structs do not have this problem.

template <class T>
struct test_is_function {
    static_assert( std::is_function<T>::value, "");
    static_assert( std::is_function<const T>::value, "");
    static_assert( std::is_function<volatile T>::value, "");
    static_assert( std::is_function<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert( std::is_function_v<T>, "");
    static_assert( std::is_function_v<const T>, "");
    static_assert( std::is_function_v<volatile T>, "");
    static_assert( std::is_function_v<const volatile T>, "");
#endif
};

template <class T>
struct test_is_not_function {
    static_assert(!std::is_function<T>::value, "");
    static_assert(!std::is_function<const T>::value, "");
    static_assert(!std::is_function<volatile T>::value, "");
    static_assert(!std::is_function<const volatile T>::value, "");
#if TEST_STD_VER > 14
    static_assert(!std::is_function_v<T>, "");
    static_assert(!std::is_function_v<const T>, "");
    static_assert(!std::is_function_v<volatile T>, "");
    static_assert(!std::is_function_v<const volatile T>, "");
#endif
};

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
    test_is_function<void(void)>();
    test_is_function<int(int)>();
    test_is_function<int(int, double)>();
    test_is_function<int(Abstract *)>();
    test_is_function<void(...)>();

  test_is_not_function<std::nullptr_t>();
  test_is_not_function<void>();
  test_is_not_function<int>();
  test_is_not_function<int&>();
  test_is_not_function<int&&>();
  test_is_not_function<int*>();
  test_is_not_function<double>();
  test_is_not_function<char[3]>();
  test_is_not_function<char[]>();
  test_is_not_function<Union>();
  test_is_not_function<Enum>();
  test_is_not_function<FunctionPtr>(); // function pointer is not a function
  test_is_not_function<Empty>();
  test_is_not_function<bit_zero>();
  test_is_not_function<NotEmpty>();
  test_is_not_function<Abstract>();
  test_is_not_function<Abstract*>();
  test_is_not_function<incomplete_type>();

#if TEST_STD_VER >= 11
  test_is_function<void() noexcept>();
  test_is_function<void() const && noexcept>();
#endif
}
