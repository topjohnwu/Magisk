//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// type_traits

// member_function_pointer

#include <type_traits>
#include "test_macros.h"

// NOTE: On Windows the function `test_is_member_function<void()>` and
// `test_is_member_function<void() noexcept> has the same mangled despite being
// a distinct instantiation. This causes Clang to emit an error. However
// structs do not have this problem.
template <class T>
struct test_member_function_pointer_imp {
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
    static_assert( std::is_member_function_pointer<T>::value, "");
    static_assert(!std::is_enum<T>::value, "");
    static_assert(!std::is_union<T>::value, "");
    static_assert(!std::is_class<T>::value, "");
    static_assert(!std::is_function<T>::value, "");
};

template <class T>
struct test_member_function_pointer :
    test_member_function_pointer_imp<T>,
    test_member_function_pointer_imp<const T>,
    test_member_function_pointer_imp<volatile T>,
    test_member_function_pointer_imp<const volatile T>
{
};

class Class
{
};

struct incomplete_type;

int main()
{
  test_member_function_pointer<void (Class::*)()>();
  test_member_function_pointer<void (Class::*)(int)>();
  test_member_function_pointer<void (Class::*)(int, char)>();

  test_member_function_pointer<void (Class::*)() const>();
  test_member_function_pointer<void (Class::*)(int) const>();
  test_member_function_pointer<void (Class::*)(int, char) const>();

  test_member_function_pointer<void (Class::*)() volatile>();
  test_member_function_pointer<void (Class::*)(int) volatile>();
  test_member_function_pointer<void (Class::*)(int, char) volatile>();

  test_member_function_pointer<void (Class::*)(...)>();
  test_member_function_pointer<void (Class::*)(int, ...)>();
  test_member_function_pointer<void (Class::*)(int, char, ...)>();

  test_member_function_pointer<void (Class::*)(...) const>();
  test_member_function_pointer<void (Class::*)(int, ...) const>();
  test_member_function_pointer<void (Class::*)(int, char, ...) const>();

  test_member_function_pointer<void (Class::*)(...) volatile>();
  test_member_function_pointer<void (Class::*)(int, ...) volatile>();
  test_member_function_pointer<void (Class::*)(int, char, ...) volatile>();


// reference qualifiers on functions are a C++11 extension
#if TEST_STD_VER >= 11
  // Noexcept qualifiers
  test_member_function_pointer<void (Class::*)() noexcept>();
  test_member_function_pointer<void (Class::*)(int) noexcept>();
  test_member_function_pointer<void (Class::*)(int, char) noexcept>();

  test_member_function_pointer<void (Class::*)() const noexcept>();
  test_member_function_pointer<void (Class::*)(int) const noexcept>();
  test_member_function_pointer<void (Class::*)(int, char) const noexcept>();

  test_member_function_pointer<void (Class::*)() volatile noexcept>();
  test_member_function_pointer<void (Class::*)(int) volatile noexcept>();
  test_member_function_pointer<void (Class::*)(int, char) volatile noexcept>();

  test_member_function_pointer<void (Class::*)(...) noexcept>();
  test_member_function_pointer<void (Class::*)(int, ...) noexcept>();
  test_member_function_pointer<void (Class::*)(int, char, ...) noexcept>();

  test_member_function_pointer<void (Class::*)(...) const noexcept>();
  test_member_function_pointer<void (Class::*)(int, ...) const noexcept>();
  test_member_function_pointer<void (Class::*)(int, char, ...) const noexcept>();

  test_member_function_pointer<void (Class::*)(...) volatile noexcept>();
  test_member_function_pointer<void (Class::*)(int, ...) volatile noexcept>();
  test_member_function_pointer<void (Class::*)(int, char, ...) volatile noexcept>();

  // lvalue qualifiers
  test_member_function_pointer<void (Class::*)() &>();
  test_member_function_pointer<void (Class::*)(int) &>();
  test_member_function_pointer<void (Class::*)(int, char) &>();
  test_member_function_pointer<void (Class::*)(...) &>();
  test_member_function_pointer<void (Class::*)(int,...) &>();
  test_member_function_pointer<void (Class::*)(int, char,...) &>();

  test_member_function_pointer<void (Class::*)() const &>();
  test_member_function_pointer<void (Class::*)(int) const &>();
  test_member_function_pointer<void (Class::*)(int, char) const &>();
  test_member_function_pointer<void (Class::*)(...) const &>();
  test_member_function_pointer<void (Class::*)(int,...) const &>();
  test_member_function_pointer<void (Class::*)(int, char,...) const &>();

  test_member_function_pointer<void (Class::*)() volatile &>();
  test_member_function_pointer<void (Class::*)(int) volatile &>();
  test_member_function_pointer<void (Class::*)(int, char) volatile &>();
  test_member_function_pointer<void (Class::*)(...) volatile &>();
  test_member_function_pointer<void (Class::*)(int,...) volatile &>();
  test_member_function_pointer<void (Class::*)(int, char,...) volatile &>();

  test_member_function_pointer<void (Class::*)() const volatile &>();
  test_member_function_pointer<void (Class::*)(int) const volatile &>();
  test_member_function_pointer<void (Class::*)(int, char) const volatile &>();
  test_member_function_pointer<void (Class::*)(...) const volatile &>();
  test_member_function_pointer<void (Class::*)(int,...) const volatile &>();
  test_member_function_pointer<void (Class::*)(int, char,...) const volatile &>();

  // Lvalue qualifiers with noexcept
  test_member_function_pointer<void (Class::*)() & noexcept>();
  test_member_function_pointer<void (Class::*)(int) & noexcept>();
  test_member_function_pointer<void (Class::*)(int, char) & noexcept>();
  test_member_function_pointer<void (Class::*)(...) & noexcept>();
  test_member_function_pointer<void (Class::*)(int,...) & noexcept>();
  test_member_function_pointer<void (Class::*)(int, char,...) & noexcept>();

  test_member_function_pointer<void (Class::*)() const & noexcept>();
  test_member_function_pointer<void (Class::*)(int) const & noexcept>();
  test_member_function_pointer<void (Class::*)(int, char) const & noexcept>();
  test_member_function_pointer<void (Class::*)(...) const & noexcept>();
  test_member_function_pointer<void (Class::*)(int,...) const & noexcept>();
  test_member_function_pointer<void (Class::*)(int, char,...) const & noexcept>();

  test_member_function_pointer<void (Class::*)() volatile & noexcept>();
  test_member_function_pointer<void (Class::*)(int) volatile & noexcept>();
  test_member_function_pointer<void (Class::*)(int, char) volatile & noexcept>();
  test_member_function_pointer<void (Class::*)(...) volatile & noexcept>();
  test_member_function_pointer<void (Class::*)(int,...) volatile & noexcept>();
  test_member_function_pointer<void (Class::*)(int, char,...) volatile & noexcept>();

  test_member_function_pointer<void (Class::*)() const volatile & noexcept>();
  test_member_function_pointer<void (Class::*)(int) const volatile & noexcept>();
  test_member_function_pointer<void (Class::*)(int, char) const volatile & noexcept>();
  test_member_function_pointer<void (Class::*)(...) const volatile & noexcept>();
  test_member_function_pointer<void (Class::*)(int,...) const volatile & noexcept>();
  test_member_function_pointer<void (Class::*)(int, char,...) const volatile & noexcept>();

  // RValue qualifiers
  test_member_function_pointer<void (Class::*)() &&>();
  test_member_function_pointer<void (Class::*)(int) &&>();
  test_member_function_pointer<void (Class::*)(int, char) &&>();
  test_member_function_pointer<void (Class::*)(...) &&>();
  test_member_function_pointer<void (Class::*)(int,...) &&>();
  test_member_function_pointer<void (Class::*)(int, char,...) &&>();

  test_member_function_pointer<void (Class::*)() const &&>();
  test_member_function_pointer<void (Class::*)(int) const &&>();
  test_member_function_pointer<void (Class::*)(int, char) const &&>();
  test_member_function_pointer<void (Class::*)(...) const &&>();
  test_member_function_pointer<void (Class::*)(int,...) const &&>();
  test_member_function_pointer<void (Class::*)(int, char,...) const &&>();

  test_member_function_pointer<void (Class::*)() volatile &&>();
  test_member_function_pointer<void (Class::*)(int) volatile &&>();
  test_member_function_pointer<void (Class::*)(int, char) volatile &&>();
  test_member_function_pointer<void (Class::*)(...) volatile &&>();
  test_member_function_pointer<void (Class::*)(int,...) volatile &&>();
  test_member_function_pointer<void (Class::*)(int, char,...) volatile &&>();

  test_member_function_pointer<void (Class::*)() const volatile &&>();
  test_member_function_pointer<void (Class::*)(int) const volatile &&>();
  test_member_function_pointer<void (Class::*)(int, char) const volatile &&>();
  test_member_function_pointer<void (Class::*)(...) const volatile &&>();
  test_member_function_pointer<void (Class::*)(int,...) const volatile &&>();
  test_member_function_pointer<void (Class::*)(int, char,...) const volatile &&>();

  // RValue qualifiers with noexcept
  test_member_function_pointer<void (Class::*)() && noexcept>();
  test_member_function_pointer<void (Class::*)(int) && noexcept>();
  test_member_function_pointer<void (Class::*)(int, char) && noexcept>();
  test_member_function_pointer<void (Class::*)(...) && noexcept>();
  test_member_function_pointer<void (Class::*)(int,...) && noexcept>();
  test_member_function_pointer<void (Class::*)(int, char,...) && noexcept>();

  test_member_function_pointer<void (Class::*)() const && noexcept>();
  test_member_function_pointer<void (Class::*)(int) const && noexcept>();
  test_member_function_pointer<void (Class::*)(int, char) const && noexcept>();
  test_member_function_pointer<void (Class::*)(...) const && noexcept>();
  test_member_function_pointer<void (Class::*)(int,...) const && noexcept>();
  test_member_function_pointer<void (Class::*)(int, char,...) const && noexcept>();

  test_member_function_pointer<void (Class::*)() volatile && noexcept>();
  test_member_function_pointer<void (Class::*)(int) volatile && noexcept>();
  test_member_function_pointer<void (Class::*)(int, char) volatile && noexcept>();
  test_member_function_pointer<void (Class::*)(...) volatile && noexcept>();
  test_member_function_pointer<void (Class::*)(int,...) volatile && noexcept>();
  test_member_function_pointer<void (Class::*)(int, char,...) volatile && noexcept>();

  test_member_function_pointer<void (Class::*)() const volatile && noexcept>();
  test_member_function_pointer<void (Class::*)(int) const volatile && noexcept>();
  test_member_function_pointer<void (Class::*)(int, char) const volatile && noexcept>();
  test_member_function_pointer<void (Class::*)(...) const volatile && noexcept>();
  test_member_function_pointer<void (Class::*)(int,...) const volatile && noexcept>();
  test_member_function_pointer<void (Class::*)(int, char,...) const volatile && noexcept>();
#endif

//  LWG#2582
  static_assert(!std::is_member_function_pointer<incomplete_type>::value, "");
}
