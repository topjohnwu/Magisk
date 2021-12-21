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

// optional<T>& operator=(optional<T>&& rhs)
//     noexcept(is_nothrow_move_assignable<T>::value &&
//              is_nothrow_move_constructible<T>::value); // constexpr in C++20

#include <optional>
#include <cassert>
#include <type_traits>
#include <utility>

#include "test_macros.h"
#include "archetypes.hpp"

using std::optional;

struct X
{
    static bool throw_now;
    static int alive;

    X() { ++alive; }
    X(X&&)
    {
        if (throw_now)
            TEST_THROW(6);
        ++alive;
    }

    X& operator=(X&&)
    {
        if (throw_now)
            TEST_THROW(42);
        return *this;
    }

    ~X() { assert(alive > 0); --alive; }
};

struct Y {};

bool X::throw_now = false;
int X::alive = 0;


template <class Tp>
constexpr bool assign_empty(optional<Tp>&& lhs) {
    optional<Tp> rhs;
    lhs = std::move(rhs);
    return !lhs.has_value() && !rhs.has_value();
}

template <class Tp>
constexpr bool assign_value(optional<Tp>&& lhs) {
    optional<Tp> rhs(101);
    lhs = std::move(rhs);
    return lhs.has_value() && rhs.has_value() && *lhs == Tp{101};
}

int main()
{
    {
        static_assert(std::is_nothrow_move_assignable<optional<int>>::value, "");
        optional<int> opt;
        constexpr optional<int> opt2;
        opt = std::move(opt2);
        static_assert(static_cast<bool>(opt2) == false, "");
        assert(static_cast<bool>(opt) == static_cast<bool>(opt2));
    }
    {
        optional<int> opt;
        constexpr optional<int> opt2(2);
        opt = std::move(opt2);
        static_assert(static_cast<bool>(opt2) == true, "");
        static_assert(*opt2 == 2, "");
        assert(static_cast<bool>(opt) == static_cast<bool>(opt2));
        assert(*opt == *opt2);
    }
    {
        optional<int> opt(3);
        constexpr optional<int> opt2;
        opt = std::move(opt2);
        static_assert(static_cast<bool>(opt2) == false, "");
        assert(static_cast<bool>(opt) == static_cast<bool>(opt2));
    }
    {
        using T = TestTypes::TestType;
        T::reset();
        optional<T> opt(3);
        optional<T> opt2;
        assert(T::alive == 1);
        opt = std::move(opt2);
        assert(T::alive == 0);
        assert(static_cast<bool>(opt2) == false);
        assert(static_cast<bool>(opt) == static_cast<bool>(opt2));
    }
    {
        optional<int> opt(3);
        constexpr optional<int> opt2(2);
        opt = std::move(opt2);
        static_assert(static_cast<bool>(opt2) == true, "");
        static_assert(*opt2 == 2, "");
        assert(static_cast<bool>(opt) == static_cast<bool>(opt2));
        assert(*opt == *opt2);
    }
    {
        using O = optional<int>;
#if TEST_STD_VER > 17
        LIBCPP_STATIC_ASSERT(assign_empty(O{42}), "");
        LIBCPP_STATIC_ASSERT(assign_value(O{42}), "");
#endif
        assert(assign_empty(O{42}));
        assert(assign_value(O{42}));
    }
    {
        using O = optional<TrivialTestTypes::TestType>;
#if TEST_STD_VER > 17
        LIBCPP_STATIC_ASSERT(assign_empty(O{42}), "");
        LIBCPP_STATIC_ASSERT(assign_value(O{42}), "");
#endif
        assert(assign_empty(O{42}));
        assert(assign_value(O{42}));
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        static_assert(!std::is_nothrow_move_assignable<optional<X>>::value, "");
        X::alive = 0;
        X::throw_now = false;
        optional<X> opt;
        optional<X> opt2(X{});
        assert(X::alive == 1);
        assert(static_cast<bool>(opt2) == true);
        try
        {
            X::throw_now = true;
            opt = std::move(opt2);
            assert(false);
        }
        catch (int i)
        {
            assert(i == 6);
            assert(static_cast<bool>(opt) == false);
        }
        assert(X::alive == 1);
    }
    assert(X::alive == 0);
    {
        static_assert(!std::is_nothrow_move_assignable<optional<X>>::value, "");
        X::throw_now = false;
        optional<X> opt(X{});
        optional<X> opt2(X{});
        assert(X::alive == 2);
        assert(static_cast<bool>(opt2) == true);
        try
        {
            X::throw_now = true;
            opt = std::move(opt2);
            assert(false);
        }
        catch (int i)
        {
            assert(i == 42);
            assert(static_cast<bool>(opt) == true);
        }
        assert(X::alive == 2);
    }
    assert(X::alive == 0);
#endif // TEST_HAS_NO_EXCEPTIONS
    {
        static_assert(std::is_nothrow_move_assignable<optional<Y>>::value, "");
    }
    {
        struct ThrowsMove {
            ThrowsMove() noexcept {}
            ThrowsMove(ThrowsMove const&) noexcept {}
            ThrowsMove(ThrowsMove &&) noexcept(false) {}
            ThrowsMove& operator=(ThrowsMove const&) noexcept { return *this; }
            ThrowsMove& operator=(ThrowsMove &&) noexcept { return *this; }
        };
        static_assert(!std::is_nothrow_move_assignable<optional<ThrowsMove>>::value, "");
        struct ThrowsMoveAssign {
            ThrowsMoveAssign() noexcept {}
            ThrowsMoveAssign(ThrowsMoveAssign const&) noexcept {}
            ThrowsMoveAssign(ThrowsMoveAssign &&) noexcept {}
            ThrowsMoveAssign& operator=(ThrowsMoveAssign const&) noexcept { return *this; }
            ThrowsMoveAssign& operator=(ThrowsMoveAssign &&) noexcept(false) { return *this; }
        };
        static_assert(!std::is_nothrow_move_assignable<optional<ThrowsMoveAssign>>::value, "");
        struct NoThrowMove {
            NoThrowMove() noexcept(false) {}
            NoThrowMove(NoThrowMove const&) noexcept(false) {}
            NoThrowMove(NoThrowMove &&) noexcept {}
            NoThrowMove& operator=(NoThrowMove const&) noexcept { return *this; }
            NoThrowMove& operator=(NoThrowMove&&) noexcept { return *this; }
        };
        static_assert(std::is_nothrow_move_assignable<optional<NoThrowMove>>::value, "");
    }
}
