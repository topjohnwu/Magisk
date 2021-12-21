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

// template <class Value> any(Value &&)

// Test construction from a value.
// Concerns:
// ---------
// 1. The value is properly move/copied depending on the value category.
// 2. Both small and large values are properly handled.


#include <any>
#include <cassert>

#include "any_helpers.h"
#include "count_new.hpp"
#include "test_macros.h"

using std::any;
using std::any_cast;

template <class Type>
void test_copy_value_throws()
{
#if !defined(TEST_HAS_NO_EXCEPTIONS)
    assert(Type::count == 0);
    {
        Type const t(42);
        assert(Type::count == 1);
        try {
            any const a2(t);
            assert(false);
        } catch (my_any_exception const &) {
            // do nothing
        } catch (...) {
            assert(false);
        }
        assert(Type::count == 1);
        assert(t.value == 42);
    }
    assert(Type::count == 0);
#endif
}

void test_move_value_throws()
{
#if !defined(TEST_HAS_NO_EXCEPTIONS)
    assert(throws_on_move::count == 0);
    {
        throws_on_move v;
        assert(throws_on_move::count == 1);
        try {
            any const a(std::move(v));
            assert(false);
        } catch (my_any_exception const &) {
            // do nothing
        } catch (...) {
            assert(false);
        }
        assert(throws_on_move::count == 1);
    }
    assert(throws_on_move::count == 0);
#endif
}

template <class Type>
void test_copy_move_value() {
    // constructing from a small type should perform no allocations.
    DisableAllocationGuard g(isSmallType<Type>()); ((void)g);
    assert(Type::count == 0);
    Type::reset();
    {
        Type t(42);
        assert(Type::count == 1);

        any a(t);

        assert(Type::count == 2);
        assert(Type::copied == 1);
        assert(Type::moved == 0);
        assertContains<Type>(a, 42);
    }
    assert(Type::count == 0);
    Type::reset();
    {
        Type t(42);
        assert(Type::count == 1);

        any a(std::move(t));

        assert(Type::count == 2);
        assert(Type::copied == 0);
        assert(Type::moved == 1);
        assertContains<Type>(a, 42);
    }
}

// Test that any(ValueType&&) is *never* selected for a std::in_place_type_t specialization.
void test_sfinae_constraints() {
    using BadTag = std::in_place_type_t<int>;
    using OKTag = std::in_place_t;
    // Test that the tag type is properly handled in SFINAE
    BadTag t = std::in_place_type<int>;
    OKTag ot = std::in_place;
    {
        std::any a(t);
        assertContains<int>(a, 0);
    }
    {
        std::any a(std::move(t));
        assertContains<int>(a, 0);
    }
    {
        std::any a(ot);
        assert(containsType<OKTag>(a));
    }
    {
        struct Dummy { Dummy() = delete; };
        using T = std::in_place_type_t<Dummy>;
        static_assert(!std::is_constructible<std::any, T>::value, "");
    }
    {
        // Test that the ValueType&& constructor SFINAE's away when the
        // argument is non-copyable
        struct NoCopy {
          NoCopy() = default;
          NoCopy(NoCopy const&) = delete;
          NoCopy(int) {}
        };
        static_assert(!std::is_constructible<std::any, NoCopy>::value, "");
        static_assert(!std::is_constructible<std::any, NoCopy&>::value, "");
        static_assert(!std::is_convertible<NoCopy, std::any>::value, "");
    }
}

int main() {
    test_copy_move_value<small>();
    test_copy_move_value<large>();
    test_copy_value_throws<small_throws_on_copy>();
    test_copy_value_throws<large_throws_on_copy>();
    test_move_value_throws();
    test_sfinae_constraints();
}
