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

// optional<T>& operator=(const optional<T>& rhs); // constexpr in C++20

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
    X(const X&)
    {
        if (throw_now)
            TEST_THROW(6);
    }
};

bool X::throw_now = false;

template <class Tp>
constexpr bool assign_empty(optional<Tp>&& lhs) {
    const optional<Tp> rhs;
    lhs = rhs;
    return !lhs.has_value() && !rhs.has_value();
}

template <class Tp>
constexpr bool assign_value(optional<Tp>&& lhs) {
    const optional<Tp> rhs(101);
    lhs = rhs;
    return lhs.has_value() && rhs.has_value() && *lhs == *rhs;
}

int main()
{
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
    {
        using O = optional<TestTypes::TestType>;
        assert(assign_empty(O{42}));
        assert(assign_value(O{42}));
    }
    {
        using T = TestTypes::TestType;
        T::reset();
        optional<T> opt(3);
        const optional<T> opt2;
        assert(T::alive == 1);
        opt = opt2;
        assert(T::alive == 0);
        assert(!opt2.has_value());
        assert(!opt.has_value());
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        optional<X> opt;
        optional<X> opt2(X{});
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
