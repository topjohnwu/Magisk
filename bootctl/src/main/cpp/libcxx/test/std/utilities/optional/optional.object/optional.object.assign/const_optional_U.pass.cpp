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

// From LWG2451:
// template<class U>
//   optional<T>& operator=(const optional<U>& rhs);

#include <optional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "archetypes.hpp"

using std::optional;

struct X
{
    static bool throw_now;

    X() = default;
    X(int)
    {
        if (throw_now)
            TEST_THROW(6);
    }
};

bool X::throw_now = false;

struct Y1
{
    Y1() = default;
    Y1(const int&) {}
    Y1& operator=(const Y1&) = delete;
};

struct Y2
{
    Y2() = default;
    Y2(const int&) = delete;
    Y2& operator=(const int&) { return *this; }
};

template <class T>
struct AssignableFrom {
  static int type_constructed;
  static int type_assigned;
static int int_constructed;
  static int int_assigned;

  static void reset() {
      type_constructed = int_constructed = 0;
      type_assigned = int_assigned = 0;
  }

  AssignableFrom() = default;

  explicit AssignableFrom(T) { ++type_constructed; }
  AssignableFrom& operator=(T) { ++type_assigned; return *this; }

  AssignableFrom(int) { ++int_constructed; }
  AssignableFrom& operator=(int) { ++int_assigned; return *this; }
private:
  AssignableFrom(AssignableFrom const&) = delete;
  AssignableFrom& operator=(AssignableFrom const&) = delete;
};

template <class T> int AssignableFrom<T>::type_constructed = 0;
template <class T> int AssignableFrom<T>::type_assigned = 0;
template <class T> int AssignableFrom<T>::int_constructed = 0;
template <class T> int AssignableFrom<T>::int_assigned = 0;


void test_with_test_type() {
    using T = TestTypes::TestType;
    T::reset();
    { // non-empty to empty
        T::reset_constructors();
        optional<T> opt;
        const optional<int> other(42);
        opt = other;
        assert(T::alive == 1);
        assert(T::constructed == 1);
        assert(T::value_constructed == 1);
        assert(T::assigned == 0);
        assert(T::destroyed == 0);
        assert(static_cast<bool>(other) == true);
        assert(*other == 42);
        assert(static_cast<bool>(opt) == true);
        assert(*opt == T(42));
    }
    assert(T::alive == 0);
    { // non-empty to non-empty
        optional<T> opt(101);
        const optional<int> other(42);
        T::reset_constructors();
        opt = other;
        assert(T::alive == 1);
        assert(T::constructed == 0);
        assert(T::assigned == 1);
        assert(T::value_assigned == 1);
        assert(T::destroyed == 0);
        assert(static_cast<bool>(other) == true);
        assert(*other == 42);
        assert(static_cast<bool>(opt) == true);
        assert(*opt == T(42));
    }
    assert(T::alive == 0);
    { // empty to non-empty
        optional<T> opt(101);
        const optional<int> other;
        T::reset_constructors();
        opt = other;
        assert(T::alive == 0);
        assert(T::constructed == 0);
        assert(T::assigned == 0);
        assert(T::destroyed == 1);
        assert(static_cast<bool>(other) == false);
        assert(static_cast<bool>(opt) == false);
    }
    assert(T::alive == 0);
    { // empty to empty
        optional<T> opt;
        const optional<int> other;
        T::reset_constructors();
        opt = other;
        assert(T::alive == 0);
        assert(T::constructed == 0);
        assert(T::assigned == 0);
        assert(T::destroyed == 0);
        assert(static_cast<bool>(other) == false);
        assert(static_cast<bool>(opt) == false);
    }
    assert(T::alive == 0);
}

void test_ambigious_assign() {
    using OptInt = std::optional<int>;
    {
        using T = AssignableFrom<OptInt const&>;
        const OptInt a(42);
        T::reset();
        {
            std::optional<T> t;
            t = a;
            assert(T::type_constructed == 1);
            assert(T::type_assigned == 0);
            assert(T::int_constructed == 0);
            assert(T::int_assigned == 0);
        }
        T::reset();
        {
            std::optional<T> t(42);
            t = a;
            assert(T::type_constructed == 0);
            assert(T::type_assigned == 1);
            assert(T::int_constructed == 1);
            assert(T::int_assigned == 0);
        }
        T::reset();
        {
            std::optional<T> t(42);
            t = std::move(a);
            assert(T::type_constructed == 0);
            assert(T::type_assigned == 1);
            assert(T::int_constructed == 1);
            assert(T::int_assigned == 0);
        }
    }
    {
        using T = AssignableFrom<OptInt&>;
        OptInt a(42);
        T::reset();
        {
            std::optional<T> t;
            t = a;
            assert(T::type_constructed == 1);
            assert(T::type_assigned == 0);
            assert(T::int_constructed == 0);
            assert(T::int_assigned == 0);
        }
        {
            using Opt = std::optional<T>;
            static_assert(!std::is_assignable_v<Opt&, OptInt const&>, "");
        }
    }
}


int main()
{
    test_with_test_type();
    test_ambigious_assign();
    {
        optional<int> opt;
        constexpr optional<short> opt2;
        opt = opt2;
        static_assert(static_cast<bool>(opt2) == false, "");
        assert(static_cast<bool>(opt) == static_cast<bool>(opt2));
    }
    {
        optional<int> opt;
        constexpr optional<short> opt2(short{2});
        opt = opt2;
        static_assert(static_cast<bool>(opt2) == true, "");
        static_assert(*opt2 == 2, "");
        assert(static_cast<bool>(opt) == static_cast<bool>(opt2));
        assert(*opt == *opt2);
    }
    {
        optional<int> opt(3);
        constexpr optional<short> opt2;
        opt = opt2;
        static_assert(static_cast<bool>(opt2) == false, "");
        assert(static_cast<bool>(opt) == static_cast<bool>(opt2));
    }
    {
        optional<int> opt(3);
        constexpr optional<short> opt2(short{2});
        opt = opt2;
        static_assert(static_cast<bool>(opt2) == true, "");
        static_assert(*opt2 == 2, "");
        assert(static_cast<bool>(opt) == static_cast<bool>(opt2));
        assert(*opt == *opt2);
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        optional<X> opt;
        optional<int> opt2(42);
        assert(static_cast<bool>(opt2) == true);
        try
        {
            X::throw_now = true;
            opt = opt2;
            assert(false);
        }
        catch (int i)
        {
            assert(i == 6);
            assert(static_cast<bool>(opt) == false);
        }
    }
#endif
}
