//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14
// <optional>

// template <class U> optional<T>& operator=(U&& v);

#include <optional>
#include <type_traits>
#include <cassert>
#include <memory>

#include "test_macros.h"
#include "archetypes.hpp"

using std::optional;

struct ThrowAssign {
  static int dtor_called;
  ThrowAssign() = default;
  ThrowAssign(int) { TEST_THROW(42); }
  ThrowAssign& operator=(int) {
      TEST_THROW(42);
  }
  ~ThrowAssign() { ++dtor_called; }
};
int ThrowAssign::dtor_called = 0;

template <class T, class Arg = T, bool Expect = true>
void assert_assignable() {
    static_assert(std::is_assignable<optional<T>&, Arg>::value == Expect, "");
    static_assert(!std::is_assignable<const optional<T>&, Arg>::value, "");
}

struct MismatchType {
  explicit MismatchType(int) {}
  explicit MismatchType(char*) {}
  explicit MismatchType(int*) = delete;
  MismatchType& operator=(int) { return *this; }
  MismatchType& operator=(int*) { return *this; }
  MismatchType& operator=(char*) = delete;
};

struct FromOptionalType {
  using Opt = std::optional<FromOptionalType>;
  FromOptionalType() = default;
  FromOptionalType(FromOptionalType const&) = delete;
  template <class Dummy = void>
  constexpr FromOptionalType(Opt&) { Dummy::BARK; }
  template <class Dummy = void>
  constexpr FromOptionalType& operator=(Opt&) { Dummy::BARK; return *this; }
};

void test_sfinae() {
    using I = TestTypes::TestType;
    using E = ExplicitTestTypes::TestType;
    assert_assignable<int>();
    assert_assignable<int, int&>();
    assert_assignable<int, int const&>();
    // Implicit test type
    assert_assignable<I, I const&>();
    assert_assignable<I, I&&>();
    assert_assignable<I, int>();
    assert_assignable<I, void*, false>();
    // Explicit test type
    assert_assignable<E, E const&>();
    assert_assignable<E, E &&>();
    assert_assignable<E, int>();
    assert_assignable<E, void*, false>();
    // Mismatch type
    assert_assignable<MismatchType, int>();
    assert_assignable<MismatchType, int*, false>();
    assert_assignable<MismatchType, char*, false>();
    // Type constructible from optional
    assert_assignable<FromOptionalType, std::optional<FromOptionalType>&, false>();
}

void test_with_test_type()
{
    using T = TestTypes::TestType;
    T::reset();
    { // to empty
        optional<T> opt;
        opt = 3;
        assert(T::alive == 1);
        assert(T::constructed == 1);
        assert(T::value_constructed == 1);
        assert(T::assigned == 0);
        assert(T::destroyed == 0);
        assert(static_cast<bool>(opt) == true);
        assert(*opt == T(3));
    }
    { // to existing
        optional<T> opt(42);
        T::reset_constructors();
        opt = 3;
        assert(T::alive == 1);
        assert(T::constructed == 0);
        assert(T::assigned == 1);
        assert(T::value_assigned == 1);
        assert(T::destroyed == 0);
        assert(static_cast<bool>(opt) == true);
        assert(*opt == T(3));
    }
    { // test default argument
        optional<T> opt;
        T::reset_constructors();
        opt = {1, 2};
        assert(T::alive == 1);
        assert(T::constructed == 2);
        assert(T::value_constructed == 1);
        assert(T::move_constructed == 1);
        assert(T::assigned == 0);
        assert(T::destroyed == 1);
        assert(static_cast<bool>(opt) == true);
        assert(*opt == T(1, 2));
    }
    { // test default argument
        optional<T> opt(42);
        T::reset_constructors();
        opt = {1, 2};
        assert(T::alive == 1);
        assert(T::constructed == 1);
        assert(T::value_constructed == 1);
        assert(T::assigned == 1);
        assert(T::move_assigned == 1);
        assert(T::destroyed == 1);
        assert(static_cast<bool>(opt) == true);
        assert(*opt == T(1, 2));
    }
    { // test default argument
        optional<T> opt;
        T::reset_constructors();
        opt = {1};
        assert(T::alive == 1);
        assert(T::constructed == 2);
        assert(T::value_constructed == 1);
        assert(T::move_constructed == 1);
        assert(T::assigned == 0);
        assert(T::destroyed == 1);
        assert(static_cast<bool>(opt) == true);
        assert(*opt == T(1));
    }
    { // test default argument
        optional<T> opt(42);
        T::reset_constructors();
        opt = {};
        assert(static_cast<bool>(opt) == false);
        assert(T::alive == 0);
        assert(T::constructed == 0);
        assert(T::assigned == 0);
        assert(T::destroyed == 1);
    }
}

template <class T, class Value = int>
void test_with_type() {
    { // to empty
        optional<T> opt;
        opt = Value(3);
        assert(static_cast<bool>(opt) == true);
        assert(*opt == T(3));
    }
    { // to existing
        optional<T> opt(Value(42));
        opt = Value(3);
        assert(static_cast<bool>(opt) == true);
        assert(*opt == T(3));
    }
    { // test const
        optional<T> opt(Value(42));
        const T t(Value(3));
        opt = t;
        assert(static_cast<bool>(opt) == true);
        assert(*opt == T(3));
    }
    { // test default argument
        optional<T> opt;
        opt = {Value(1)};
        assert(static_cast<bool>(opt) == true);
        assert(*opt == T(1));
    }
    { // test default argument
        optional<T> opt(Value(42));
        opt = {};
        assert(static_cast<bool>(opt) == false);
    }
}

template <class T>
void test_with_type_multi() {
    test_with_type<T>();
    { // test default argument
        optional<T> opt;
        opt = {1, 2};
        assert(static_cast<bool>(opt) == true);
        assert(*opt == T(1, 2));
    }
    { // test default argument
        optional<T> opt(42);
        opt = {1, 2};
        assert(static_cast<bool>(opt) == true);
        assert(*opt == T(1, 2));
    }
}

void test_throws()
{
#ifndef TEST_HAS_NO_EXCEPTIONS
    using T = ThrowAssign;
    {
        optional<T> opt;
        try {
            opt = 42;
            assert(false);
        } catch (int) {}
        assert(static_cast<bool>(opt) == false);
    }
    assert(T::dtor_called == 0);
    {
        T::dtor_called = 0;
        optional<T> opt(std::in_place);
        try {
            opt = 42;
            assert(false);
        } catch (int) {}
        assert(static_cast<bool>(opt) == true);
        assert(T::dtor_called == 0);
    }
    assert(T::dtor_called == 1);
#endif
}

enum MyEnum { Zero, One, Two, Three, FortyTwo = 42 };

using Fn = void(*)();

int main()
{
    test_sfinae();
    // Test with instrumented type
    test_with_test_type();
    // Test with various scalar types
    test_with_type<int>();
    test_with_type<MyEnum, MyEnum>();
    test_with_type<int, MyEnum>();
    test_with_type<Fn, Fn>();
    // Test types with multi argument constructors
    test_with_type_multi<ConstexprTestTypes::TestType>();
    test_with_type_multi<TrivialTestTypes::TestType>();
    // Test move only types
    {
        optional<std::unique_ptr<int>> opt;
        opt = std::unique_ptr<int>(new int(3));
        assert(static_cast<bool>(opt) == true);
        assert(**opt == 3);
    }
    {
        optional<std::unique_ptr<int>> opt(std::unique_ptr<int>(new int(2)));
        opt = std::unique_ptr<int>(new int(3));
        assert(static_cast<bool>(opt) == true);
        assert(**opt == 3);
    }
    test_throws();
}
