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

// any(any const &);

#include <any>
#include <cassert>

#include "any_helpers.h"
#include "count_new.hpp"
#include "test_macros.h"

using std::any;
using std::any_cast;

template <class Type>
void test_copy_throws() {
#if !defined(TEST_HAS_NO_EXCEPTIONS)
    assert(Type::count == 0);
    {
        any const a((Type(42)));
        assert(Type::count == 1);
        try {
            any const a2(a);
            assert(false);
        } catch (my_any_exception const &) {
            // do nothing
        } catch (...) {
            assert(false);
        }
        assert(Type::count == 1);
        assertContains<Type>(a, 42);
    }
    assert(Type::count == 0);
#endif
}

void test_copy_empty() {
    DisableAllocationGuard g; ((void)g); // No allocations should occur.
    any a1;
    any a2(a1);

    assertEmpty(a1);
    assertEmpty(a2);
}

template <class Type>
void test_copy()
{
    // Copying small types should not perform any allocations.
    DisableAllocationGuard g(isSmallType<Type>()); ((void)g);
    assert(Type::count == 0);
    Type::reset();
    {
        any a((Type(42)));
        assert(Type::count == 1);
        assert(Type::copied == 0);

        any a2(a);

        assert(Type::copied == 1);
        assert(Type::count == 2);
        assertContains<Type>(a, 42);
        assertContains<Type>(a2, 42);

        // Modify a and check that a2 is unchanged
        modifyValue<Type>(a, -1);
        assertContains<Type>(a, -1);
        assertContains<Type>(a2, 42);

        // modify a2 and check that a is unchanged
        modifyValue<Type>(a2, 999);
        assertContains<Type>(a, -1);
        assertContains<Type>(a2, 999);

        // clear a and check that a2 is unchanged
        a.reset();
        assertEmpty(a);
        assertContains<Type>(a2, 999);
    }
    assert(Type::count == 0);
}

int main() {
    test_copy<small>();
    test_copy<large>();
    test_copy_empty();
    test_copy_throws<small_throws_on_copy>();
    test_copy_throws<large_throws_on_copy>();
}
