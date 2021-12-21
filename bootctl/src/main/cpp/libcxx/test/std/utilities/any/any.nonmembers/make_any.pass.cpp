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

// template <class T, class ...Args> any make_any(Args&&...);
// template <class T, class U, class ...Args>
// any make_any(initializer_list<U>, Args&&...);

#include <any>
#include <cassert>

#include "any_helpers.h"
#include "count_new.hpp"
#include "test_macros.h"

using std::any;
using std::any_cast;


template <class Type>
void test_make_any_type() {
    // constructing from a small type should perform no allocations.
    DisableAllocationGuard g(isSmallType<Type>()); ((void)g);
    assert(Type::count == 0);
    Type::reset();
    {
        any a = std::make_any<Type>();

        assert(Type::count == 1);
        assert(Type::copied == 0);
        assert(Type::moved == 0);
        assertContains<Type>(a, 0);
    }
    assert(Type::count == 0);
    Type::reset();
    {
        any a = std::make_any<Type>(101);

        assert(Type::count == 1);
        assert(Type::copied == 0);
        assert(Type::moved == 0);
        assertContains<Type>(a, 101);
    }
    assert(Type::count == 0);
    Type::reset();
    {
        any a = std::make_any<Type>(-1, 42, -1);

        assert(Type::count == 1);
        assert(Type::copied == 0);
        assert(Type::moved == 0);
        assertContains<Type>(a, 42);
    }
    assert(Type::count == 0);
    Type::reset();
}

template <class Type>
void test_make_any_type_tracked() {
    // constructing from a small type should perform no allocations.
    DisableAllocationGuard g(isSmallType<Type>()); ((void)g);
    {
        any a = std::make_any<Type>();
        assertArgsMatch<Type>(a);
    }
    {
        any a = std::make_any<Type>(-1, 42, -1);
        assertArgsMatch<Type, int, int, int>(a);
    }
    // initializer_list constructor tests
    {
        any a = std::make_any<Type>({-1, 42, -1});
        assertArgsMatch<Type, std::initializer_list<int>>(a);
    }
    {
        int x = 42;
        any a  = std::make_any<Type>({-1, 42, -1}, x);
        assertArgsMatch<Type, std::initializer_list<int>, int&>(a);
    }
}

#ifndef TEST_HAS_NO_EXCEPTIONS

struct SmallThrows {
  SmallThrows(int) { throw 42; }
  SmallThrows(std::initializer_list<int>, int) { throw 42; }
};
static_assert(IsSmallObject<SmallThrows>::value, "");

struct LargeThrows {
  LargeThrows(int) { throw 42; }
  LargeThrows(std::initializer_list<int>, int) { throw 42; }
  int data[sizeof(std::any)];
};
static_assert(!IsSmallObject<LargeThrows>::value, "");

template <class Type>
void test_make_any_throws()
{
    {
        try {
            TEST_IGNORE_NODISCARD std::make_any<Type>(101);
            assert(false);
        } catch (int const&) {
        }
    }
    {
        try {
            TEST_IGNORE_NODISCARD std::make_any<Type>({1, 2, 3}, 101);
            assert(false);
        } catch (int const&) {
        }
    }
}

#endif

int main() {
    test_make_any_type<small>();
    test_make_any_type<large>();
    test_make_any_type<small_throws_on_copy>();
    test_make_any_type<large_throws_on_copy>();
    test_make_any_type<throws_on_move>();
    test_make_any_type_tracked<small_tracked_t>();
    test_make_any_type_tracked<large_tracked_t>();
#ifndef TEST_HAS_NO_EXCEPTIONS
    test_make_any_throws<SmallThrows>();
    test_make_any_throws<LargeThrows>();

#endif
}
