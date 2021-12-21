//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: c++98, c++03, c++11, c++14

// XFAIL: availability=macosx10.13
// XFAIL: availability=macosx10.12
// XFAIL: availability=macosx10.11
// XFAIL: availability=macosx10.10
// XFAIL: availability=macosx10.9
// XFAIL: availability=macosx10.8
// XFAIL: availability=macosx10.7

// <optional>

// constexpr optional(const T& v);

#include <optional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "archetypes.hpp"

using std::optional;

int main()
{
    {
        typedef int T;
        constexpr T t(5);
        constexpr optional<T> opt(t);
        static_assert(static_cast<bool>(opt) == true, "");
        static_assert(*opt == 5, "");

        struct test_constexpr_ctor
            : public optional<T>
        {
            constexpr test_constexpr_ctor(const T&) {}
        };

    }
    {
        typedef double T;
        constexpr T t(3);
        constexpr optional<T> opt(t);
        static_assert(static_cast<bool>(opt) == true, "");
        static_assert(*opt == 3, "");

        struct test_constexpr_ctor
            : public optional<T>
        {
            constexpr test_constexpr_ctor(const T&) {}
        };

    }
    {
        const int x = 42;
        optional<const int> o(x);
        assert(*o == x);
    }
    {
        typedef TestTypes::TestType T;
        T::reset();
        const T t(3);
        optional<T> opt = t;
        assert(T::alive == 2);
        assert(T::copy_constructed == 1);
        assert(static_cast<bool>(opt) == true);
        assert(opt.value().value == 3);
    }
    {
        typedef ExplicitTestTypes::TestType T;
        static_assert(!std::is_convertible<T const&, optional<T>>::value, "");
        T::reset();
        const T t(3);
        optional<T> opt(t);
        assert(T::alive == 2);
        assert(T::copy_constructed == 1);
        assert(static_cast<bool>(opt) == true);
        assert(opt.value().value == 3);
    }
    {
        typedef ConstexprTestTypes::TestType T;
        constexpr T t(3);
        constexpr optional<T> opt = {t};
        static_assert(static_cast<bool>(opt) == true, "");
        static_assert(opt.value().value == 3, "");

        struct test_constexpr_ctor
            : public optional<T>
        {
            constexpr test_constexpr_ctor(const T&) {}
        };
    }
    {
        typedef ExplicitConstexprTestTypes::TestType T;
        static_assert(!std::is_convertible<const T&, optional<T>>::value, "");
        constexpr T t(3);
        constexpr optional<T> opt(t);
        static_assert(static_cast<bool>(opt) == true, "");
        static_assert(opt.value().value == 3, "");

        struct test_constexpr_ctor
            : public optional<T>
        {
            constexpr test_constexpr_ctor(const T&) {}
        };

    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        struct Z {
            Z(int) {}
            Z(const Z&) {throw 6;}
        };
        typedef Z T;
        try
        {
            const T t(3);
            optional<T> opt(t);
            assert(false);
        }
        catch (int i)
        {
            assert(i == 6);
        }
    }
#endif
}
