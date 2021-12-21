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
// XFAIL: availability=macosx10.7
// XFAIL: availability=macosx10.8

// <any>

// any& operator=(any const &);

// Test copy assignment

#include <any>
#include <cassert>

#include "any_helpers.h"
#include "count_new.hpp"
#include "test_macros.h"

using std::any;
using std::any_cast;

template <class LHS, class RHS>
void test_copy_assign() {
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
}

template <class LHS>
void test_copy_assign_empty() {
    assert(LHS::count == 0);
    LHS::reset();
    {
        any lhs;
        any const rhs(LHS(42));

        assert(LHS::count == 1);
        assert(LHS::copied == 0);

        lhs = rhs;

        assert(LHS::copied == 1);
        assert(LHS::count == 2);

        assertContains<LHS>(lhs, 42);
        assertContains<LHS>(rhs, 42);
    }
    assert(LHS::count == 0);
    LHS::reset();
    {
        any lhs(LHS(1));
        any const rhs;

        assert(LHS::count == 1);
        assert(LHS::copied == 0);

        lhs = rhs;

        assert(LHS::copied == 0);
        assert(LHS::count == 0);

        assertEmpty<LHS>(lhs);
        assertEmpty(rhs);
    }
    assert(LHS::count == 0);
}

void test_copy_assign_self() {
    // empty
    {
        any a;
        a = (any &)a;
        assertEmpty(a);
        assert(globalMemCounter.checkOutstandingNewEq(0));
    }
    assert(globalMemCounter.checkOutstandingNewEq(0));
    // small
    {
        any a((small(1)));
        assert(small::count == 1);

        a = (any &)a;

        assert(small::count == 1);
        assertContains<small>(a, 1);
        assert(globalMemCounter.checkOutstandingNewEq(0));
    }
    assert(small::count == 0);
    assert(globalMemCounter.checkOutstandingNewEq(0));
    // large
    {
        any a(large(1));
        assert(large::count == 1);

        a = (any &)a;

        assert(large::count == 1);
        assertContains<large>(a, 1);
        assert(globalMemCounter.checkOutstandingNewEq(1));
    }
    assert(large::count == 0);
    assert(globalMemCounter.checkOutstandingNewEq(0));
}

template <class Tp>
void test_copy_assign_throws()
{
#if !defined(TEST_HAS_NO_EXCEPTIONS)
    auto try_throw =
    [](any& lhs, any const& rhs) {
        try {
            lhs = rhs;
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
        any const rhs((Tp(1)));
        assert(Tp::count == 1);

        try_throw(lhs, rhs);

        assert(Tp::count == 1);
        assertEmpty<Tp>(lhs);
        assertContains<Tp>(rhs, 1);
    }
    {
        any lhs((small(2)));
        any const rhs((Tp(1)));
        assert(small::count == 1);
        assert(Tp::count == 1);

        try_throw(lhs, rhs);

        assert(small::count == 1);
        assert(Tp::count == 1);
        assertContains<small>(lhs, 2);
        assertContains<Tp>(rhs, 1);
    }
    {
        any lhs((large(2)));
        any const rhs((Tp(1)));
        assert(large::count == 1);
        assert(Tp::count == 1);

        try_throw(lhs, rhs);

        assert(large::count == 1);
        assert(Tp::count == 1);
        assertContains<large>(lhs, 2);
        assertContains<Tp>(rhs, 1);
    }
#endif
}

int main() {
    test_copy_assign<small1, small2>();
    test_copy_assign<large1, large2>();
    test_copy_assign<small, large>();
    test_copy_assign<large, small>();
    test_copy_assign_empty<small>();
    test_copy_assign_empty<large>();
    test_copy_assign_self();
    test_copy_assign_throws<small_throws_on_copy>();
    test_copy_assign_throws<large_throws_on_copy>();
}
