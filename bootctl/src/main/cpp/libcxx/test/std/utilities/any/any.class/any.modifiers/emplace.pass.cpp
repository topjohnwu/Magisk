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

// template <class T, class ...Args> T& emplace(Args&&...);
// template <class T, class U, class ...Args>
// T& emplace(initializer_list<U>, Args&&...);

#include <any>
#include <cassert>

#include "any_helpers.h"
#include "count_new.hpp"
#include "test_macros.h"

using std::any;
using std::any_cast;

struct Tracked {
  static int count;
  Tracked()  {++count;}
  ~Tracked() { --count; }
};
int Tracked::count = 0;

template <class Type>
void test_emplace_type() {
    // constructing from a small type should perform no allocations.
    DisableAllocationGuard g(isSmallType<Type>()); ((void)g);
    assert(Type::count == 0);
    Type::reset();
    {
        any a(std::in_place_type<Tracked>);
        assert(Tracked::count == 1);

        auto &v = a.emplace<Type>();
        static_assert( std::is_same_v<Type&, decltype(v)>, "" );
        assert(&v == std::any_cast<Type>(&a));

        assert(Tracked::count == 0);
        assert(Type::count == 1);
        assert(Type::copied == 0);
        assert(Type::moved == 0);
        assertContains<Type>(a, 0);
    }
    assert(Type::count == 0);
    Type::reset();
    {
        any a(std::in_place_type<Tracked>);
        assert(Tracked::count == 1);

        auto &v = a.emplace<Type>(101);
        static_assert( std::is_same_v<Type&, decltype(v)>, "" );
        assert(&v == std::any_cast<Type>(&a));

        assert(Tracked::count == 0);
        assert(Type::count == 1);
        assert(Type::copied == 0);
        assert(Type::moved == 0);
        assertContains<Type>(a, 101);
    }
    assert(Type::count == 0);
    Type::reset();
    {
        any a(std::in_place_type<Tracked>);
        assert(Tracked::count == 1);

        auto &v = a.emplace<Type>(-1, 42, -1);
        static_assert( std::is_same_v<Type&, decltype(v)>, "" );
        assert(&v == std::any_cast<Type>(&a));

        assert(Tracked::count == 0);
        assert(Type::count == 1);
        assert(Type::copied == 0);
        assert(Type::moved == 0);
        assertContains<Type>(a, 42);
    }
    assert(Type::count == 0);
    Type::reset();
}

template <class Type>
void test_emplace_type_tracked() {
    // constructing from a small type should perform no allocations.
    DisableAllocationGuard g(isSmallType<Type>()); ((void)g);
    {
        any a(std::in_place_type<Tracked>);
        assert(Tracked::count == 1);
        auto &v = a.emplace<Type>();
        static_assert( std::is_same_v<Type&, decltype(v)>, "" );
        assert(&v == std::any_cast<Type>(&a));

        assert(Tracked::count == 0);
        assertArgsMatch<Type>(a);
    }
    {
        any a(std::in_place_type<Tracked>);
        assert(Tracked::count == 1);
        auto &v = a.emplace<Type>(-1, 42, -1);
        static_assert( std::is_same_v<Type&, decltype(v)>, "" );
        assert(&v == std::any_cast<Type>(&a));

        assert(Tracked::count == 0);
        assertArgsMatch<Type, int, int, int>(a);
    }
    // initializer_list constructor tests
    {
        any a(std::in_place_type<Tracked>);
        assert(Tracked::count == 1);
        auto &v = a.emplace<Type>({-1, 42, -1});
        static_assert( std::is_same_v<Type&, decltype(v)>, "" );
        assert(&v == std::any_cast<Type>(&a));

        assert(Tracked::count == 0);
        assertArgsMatch<Type, std::initializer_list<int>>(a);
    }
    {
        int x = 42;
        any a(std::in_place_type<Tracked>);
        assert(Tracked::count == 1);
        auto &v = a.emplace<Type>({-1, 42, -1}, x);
        static_assert( std::is_same_v<Type&, decltype(v)>, "" );
        assert(&v == std::any_cast<Type>(&a));

        assert(Tracked::count == 0);
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
void test_emplace_throws()
{
    // any stores small type
    {
        std::any a(small{42});
        assert(small::count == 1);
        try {
            auto &v = a.emplace<Type>(101);
            static_assert( std::is_same_v<Type&, decltype(v)>, "" );
            assert(false);
        } catch (int const&) {
        }
        assert(small::count == 0);
    }
    {
        std::any a(small{42});
        assert(small::count == 1);
        try {
            auto &v = a.emplace<Type>({1, 2, 3}, 101);
            static_assert( std::is_same_v<Type&, decltype(v)>, "" );
            assert(false);
        } catch (int const&) {
        }
        assert(small::count == 0);
    }
    // any stores large type
    {
        std::any a(large{42});
        assert(large::count == 1);
        try {
            auto &v = a.emplace<Type>(101);
            static_assert( std::is_same_v<Type&, decltype(v)>, "" );
            assert(false);
        } catch (int const&) {
        }
        assert(large::count == 0);
    }
    {
        std::any a(large{42});
        assert(large::count == 1);
        try {
            auto &v = a.emplace<Type>({1, 2, 3}, 101);
            static_assert( std::is_same_v<Type&, decltype(v)>, "" );
            assert(false);
        } catch (int const&) {
        }
        assert(large::count == 0);
    }
}

#endif

template <class T, class ...Args>
constexpr auto has_emplace(int)
    -> decltype(std::any{}.emplace<T>(std::declval<Args>()...), true) { return true; }

template <class ...Args>
constexpr bool has_emplace(long) { return false; }

template <class ...Args>
constexpr bool has_emplace() { return has_emplace<Args...>(0); }


template <class T, class IT, class ...Args>
constexpr auto has_emplace_init_list(int)
    -> decltype(std::any{}.emplace<T>(
        {std::declval<IT>(), std::declval<IT>(), std::declval<IT>()},
        std::declval<Args>()...), true) { return true; }

template <class ...Args>
constexpr bool has_emplace_init_list(long) { return false; }

template <class ...Args>
constexpr bool has_emplace_init_list() { return has_emplace_init_list<Args...>(0); }


void test_emplace_sfinae_constraints() {
    {
        static_assert(has_emplace<int>(), "");
        static_assert(has_emplace<int, int>(), "");
        static_assert(!has_emplace<int, int, int>(), "not constructible");
        static_assert(!has_emplace_init_list<int, int>(), "not constructible from il");
    }
    {
        static_assert(has_emplace<small>(), "");
        static_assert(has_emplace<large>(), "");
        static_assert(!has_emplace<small, void*>(), "");
        static_assert(!has_emplace<large, void*>(), "");

        static_assert(has_emplace_init_list<small, int>(), "");
        static_assert(has_emplace_init_list<large, int>(), "");
        static_assert(!has_emplace_init_list<small, void*>(), "");
        static_assert(!has_emplace_init_list<large, void*>(), "");
    }
    {
        // Test that the emplace SFINAE's away when the
        // argument is non-copyable
        struct NoCopy {
          NoCopy() = default;
          NoCopy(NoCopy const&) = delete;
          NoCopy(int) {}
          NoCopy(std::initializer_list<int>, int, int) {}
        };
        static_assert(!has_emplace<NoCopy>(), "");
        static_assert(!has_emplace<NoCopy, int>(), "");
        static_assert(!has_emplace_init_list<NoCopy, int, int, int>(), "");
        static_assert(!has_emplace<NoCopy&>(), "");
        static_assert(!has_emplace<NoCopy&, int>(), "");
        static_assert(!has_emplace_init_list<NoCopy&, int, int, int>(), "");
        static_assert(!has_emplace<NoCopy&&>(), "");
        static_assert(!has_emplace<NoCopy&&, int>(), "");
        static_assert(!has_emplace_init_list<NoCopy&&, int, int, int>(), "");

    }
}

int main() {
    test_emplace_type<small>();
    test_emplace_type<large>();
    test_emplace_type<small_throws_on_copy>();
    test_emplace_type<large_throws_on_copy>();
    test_emplace_type<throws_on_move>();
    test_emplace_type_tracked<small_tracked_t>();
    test_emplace_type_tracked<large_tracked_t>();
    test_emplace_sfinae_constraints();
#ifndef TEST_HAS_NO_EXCEPTIONS
    test_emplace_throws<SmallThrows>();
    test_emplace_throws<LargeThrows>();
#endif
}
