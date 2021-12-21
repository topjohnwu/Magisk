//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// XFAIL: availability=macosx10.13
// XFAIL: availability=macosx10.12
// XFAIL: availability=macosx10.11
// XFAIL: availability=macosx10.10
// XFAIL: availability=macosx10.9
// XFAIL: availability=macosx10.8
// XFAIL: availability=macosx10.7

// <any>

// template <class ValueType>
// any& operator=(ValueType&&);

// Test value copy and move assignment.

#include <any>
#include <cassert>

#include "any_helpers.h"
#include "count_new.hpp"
#include "test_macros.h"

using std::any;
using std::any_cast;

template <class LHS, class RHS>
void test_assign_value() {
    assert(LHS::count == 0);
    assert(RHS::count == 0);
    LHS::reset();
    RHS::reset();
    {
        any lhs(LHS(1));
        any const rhs(RHS(2));

        assert(LHS::count == 1);
        assert(RHS::count == 1);
        assert(RHS::copied == 0);

        lhs = rhs;

        assert(RHS::copied == 1);
        assert(LHS::count == 0);
        assert(RHS::count == 2);

        assertContains<RHS>(lhs, 2);
        assertContains<RHS>(rhs, 2);
    }
    assert(LHS::count == 0);
    assert(RHS::count == 0);
    LHS::reset();
    RHS::reset();
    {
        any lhs(LHS(1));
        any rhs(RHS(2));

        assert(LHS::count == 1);
        assert(RHS::count == 1);
        assert(RHS::moved == 1);

        lhs = std::move(rhs);

        assert(RHS::moved >= 1);
        assert(RHS::copied == 0);
        assert(LHS::count == 0);
        assert(RHS::count == 1 + rhs.has_value());
        LIBCPP_ASSERT(!rhs.has_value());

        assertContains<RHS>(lhs, 2);
        if (rhs.has_value())
            assertContains<RHS>(rhs, 0);
    }
    assert(LHS::count == 0);
    assert(RHS::count == 0);
}

template <class RHS>
void test_assign_value_empty() {
    assert(RHS::count == 0);
    RHS::reset();
    {
        any lhs;
        RHS rhs(42);
        assert(RHS::count == 1);
        assert(RHS::copied == 0);

        lhs = rhs;

        assert(RHS::count == 2);
        assert(RHS::copied == 1);
        assert(RHS::moved >= 0);
        assertContains<RHS>(lhs, 42);
    }
    assert(RHS::count == 0);
    RHS::reset();
    {
        any lhs;
        RHS rhs(42);
        assert(RHS::count == 1);
        assert(RHS::moved == 0);

        lhs = std::move(rhs);

        assert(RHS::count == 2);
        assert(RHS::copied == 0);
        assert(RHS::moved >= 1);
        assertContains<RHS>(lhs, 42);
    }
    assert(RHS::count == 0);
    RHS::reset();
}


template <class Tp, bool Move = false>
void test_assign_throws() {
#if !defined(TEST_HAS_NO_EXCEPTIONS)
    auto try_throw =
    [](any& lhs, Tp& rhs) {
        try {
            Move ? lhs = std::move(rhs)
                 : lhs = rhs;
            assert(false);
        } catch (my_any_exception const &) {
            // do nothing
        } catch (...) {
            assert(false);
        }
    };
    // const lvalue to empty
    {
        any lhs;
        Tp rhs(1);
        assert(Tp::count == 1);

        try_throw(lhs, rhs);

        assert(Tp::count == 1);
        assertEmpty<Tp>(lhs);
    }
    {
        any lhs((small(2)));
        Tp  rhs(1);
        assert(small::count == 1);
        assert(Tp::count == 1);

        try_throw(lhs, rhs);

        assert(small::count == 1);
        assert(Tp::count == 1);
        assertContains<small>(lhs, 2);
    }
    {
        any lhs((large(2)));
        Tp rhs(1);
        assert(large::count == 1);
        assert(Tp::count == 1);

        try_throw(lhs, rhs);

        assert(large::count == 1);
        assert(Tp::count == 1);
        assertContains<large>(lhs, 2);
    }
#endif
}


// Test that any& operator=(ValueType&&) is *never* selected for:
// * std::in_place type.
// * Non-copyable types
void test_sfinae_constraints() {
    { // Only the constructors are required to SFINAE on in_place_t
        using Tag = std::in_place_type_t<int>;
        using RawTag = std::remove_reference_t<Tag>;
        static_assert(std::is_assignable<std::any, RawTag&&>::value, "");
    }
    {
        struct Dummy { Dummy() = delete; };
        using T = std::in_place_type_t<Dummy>;
        static_assert(std::is_assignable<std::any, T>::value, "");
    }
    {
        // Test that the ValueType&& constructor SFINAE's away when the
        // argument is non-copyable
        struct NoCopy {
          NoCopy() = default;
          NoCopy(NoCopy const&) = delete;
          NoCopy(NoCopy&&) = default;
        };
        static_assert(!std::is_assignable<std::any, NoCopy>::value, "");
        static_assert(!std::is_assignable<std::any, NoCopy&>::value, "");
    }
}

int main() {
    test_assign_value<small1, small2>();
    test_assign_value<large1, large2>();
    test_assign_value<small, large>();
    test_assign_value<large, small>();
    test_assign_value_empty<small>();
    test_assign_value_empty<large>();
    test_assign_throws<small_throws_on_copy>();
    test_assign_throws<large_throws_on_copy>();
    test_assign_throws<throws_on_move, /* Move = */ true>();
    test_sfinae_constraints();
}
