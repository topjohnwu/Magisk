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

// any& operator=(any &&);

// Test move assignment.

#include <any>
#include <cassert>

#include "any_helpers.h"
#include "test_macros.h"

using std::any;
using std::any_cast;

template <class LHS, class RHS>
void test_move_assign() {
    assert(LHS::count == 0);
    assert(RHS::count == 0);
    {
        LHS const s1(1);
        any a(s1);
        RHS const s2(2);
        any a2(s2);

        assert(LHS::count == 2);
        assert(RHS::count == 2);

        a = std::move(a2);

        assert(LHS::count == 1);
        assert(RHS::count == 2 + a2.has_value());
        LIBCPP_ASSERT(RHS::count == 2); // libc++ leaves the object empty

        assertContains<RHS>(a, 2);
        if (a2.has_value())
            assertContains<RHS>(a2, 0);
        LIBCPP_ASSERT(!a2.has_value());
    }
    assert(LHS::count == 0);
    assert(RHS::count == 0);
}

template <class LHS>
void test_move_assign_empty() {
    assert(LHS::count == 0);
    {
        any a;
        any a2((LHS(1)));

        assert(LHS::count == 1);

        a = std::move(a2);

        assert(LHS::count == 1 + a2.has_value());
        LIBCPP_ASSERT(LHS::count == 1);

        assertContains<LHS>(a, 1);
        if (a2.has_value())
            assertContains<LHS>(a2, 0);
        LIBCPP_ASSERT(!a2.has_value());
    }
    assert(LHS::count == 0);
    {
        any a((LHS(1)));
        any a2;

        assert(LHS::count == 1);

        a = std::move(a2);

        assert(LHS::count == 0);

        assertEmpty<LHS>(a);
        assertEmpty(a2);
    }
    assert(LHS::count == 0);
}

void test_move_assign_noexcept() {
    any a1;
    any a2;
    static_assert(
        noexcept(a1 = std::move(a2))
      , "any & operator=(any &&) must be noexcept"
      );
}

int main() {
    test_move_assign_noexcept();
    test_move_assign<small1, small2>();
    test_move_assign<large1, large2>();
    test_move_assign<small, large>();
    test_move_assign<large, small>();
    test_move_assign_empty<small>();
    test_move_assign_empty<large>();
}
