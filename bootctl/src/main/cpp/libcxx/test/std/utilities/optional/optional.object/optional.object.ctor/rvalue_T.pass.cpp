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

// constexpr optional(T&& v);

#include <optional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "archetypes.hpp"


using std::optional;


class Z
{
public:
    Z(int) {}
    Z(Z&&) {TEST_THROW(6);}
};


int main()
{
    {
        typedef int T;
        constexpr optional<T> opt(T(5));
        static_assert(static_cast<bool>(opt) == true, "");
        static_assert(*opt == 5, "");

        struct test_constexpr_ctor
            : public optional<T>
        {
            constexpr test_constexpr_ctor(T&&) {}
        };
    }
    {
        typedef double T;
        constexpr optional<T> opt(T(3));
        static_assert(static_cast<bool>(opt) == true, "");
        static_assert(*opt == 3, "");

        struct test_constexpr_ctor
            : public optional<T>
        {
            constexpr test_constexpr_ctor(T&&) {}
        };
    }
    {
        const int x = 42;
        optional<const int> o(std::move(x));
        assert(*o == 42);
    }
    {
        typedef TestTypes::TestType T;
        T::reset();
        optional<T> opt = T{3};
        assert(T::alive == 1);
        assert(T::move_constructed == 1);
        assert(static_cast<bool>(opt) == true);
        assert(opt.value().value == 3);
    }
    {
        typedef ExplicitTestTypes::TestType T;
        static_assert(!std::is_convertible<T&&, optional<T>>::value, "");
        T::reset();
        optional<T> opt(T{3});
        assert(T::alive == 1);
        assert(T::move_constructed == 1);
        assert(static_cast<bool>(opt) == true);
        assert(opt.value().value == 3);
    }
    {
        typedef TestTypes::TestType T;
        T::reset();
        optional<T> opt = {3};
        assert(T::alive == 1);
        assert(T::value_constructed == 1);
        assert(T::copy_constructed == 0);
        assert(T::move_constructed == 0);
        assert(static_cast<bool>(opt) == true);
        assert(opt.value().value == 3);
    }
    {
        typedef ConstexprTestTypes::TestType T;
        constexpr optional<T> opt = {T(3)};
        static_assert(static_cast<bool>(opt) == true, "");
        static_assert(opt.value().value == 3, "");

        struct test_constexpr_ctor
            : public optional<T>
        {
            constexpr test_constexpr_ctor(const T&) {}
        };
    }
    {
        typedef ConstexprTestTypes::TestType T;
        constexpr optional<T> opt = {3};
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
        static_assert(!std::is_convertible<T&&, optional<T>>::value, "");
        constexpr optional<T> opt(T{3});
        static_assert(static_cast<bool>(opt) == true, "");
        static_assert(opt.value().value == 3, "");

        struct test_constexpr_ctor
            : public optional<T>
        {
            constexpr test_constexpr_ctor(T&&) {}
        };

    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        struct Z {
            Z(int) {}
            Z(Z&&) {throw 6;}
        };
        typedef Z T;
        try
        {
            T t(3);
            optional<T> opt(std::move(t));
            assert(false);
        }
        catch (int i)
        {
            assert(i == 6);
        }
    }
#endif
}
