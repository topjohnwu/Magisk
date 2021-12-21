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

// <optional>

// constexpr optional(optional<T>&& rhs);

#include <optional>
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "archetypes.hpp"

using std::optional;

template <class T, class ...InitArgs>
void test(InitArgs&&... args)
{
    const optional<T> orig(std::forward<InitArgs>(args)...);
    optional<T> rhs(orig);
    bool rhs_engaged = static_cast<bool>(rhs);
    optional<T> lhs = std::move(rhs);
    assert(static_cast<bool>(lhs) == rhs_engaged);
    if (rhs_engaged)
        assert(*lhs == *orig);
}

template <class T, class ...InitArgs>
constexpr bool constexpr_test(InitArgs&&... args)
{
    static_assert( std::is_trivially_copy_constructible_v<T>, ""); // requirement
    const optional<T> orig(std::forward<InitArgs>(args)...);
    optional<T> rhs(orig);
    optional<T> lhs = std::move(rhs);
    return (lhs.has_value() == orig.has_value()) &&
           (lhs.has_value() ? *lhs == *orig : true);
}

void test_throwing_ctor() {
#ifndef TEST_HAS_NO_EXCEPTIONS
    struct Z {
        Z() : count(0) {}
        Z(Z&& o) : count(o.count + 1)
        { if (count == 2) throw 6; }
        int count;
    };
    Z z;
    optional<Z> rhs(std::move(z));
    try
    {
        optional<Z> lhs(std::move(rhs));
        assert(false);
    }
    catch (int i)
    {
        assert(i == 6);
    }
#endif
}


template <class T, class ...InitArgs>
void test_ref(InitArgs&&... args)
{
    optional<T> rhs(std::forward<InitArgs>(args)...);
    bool rhs_engaged = static_cast<bool>(rhs);
    optional<T> lhs = std::move(rhs);
    assert(static_cast<bool>(lhs) == rhs_engaged);
    if (rhs_engaged)
        assert(&(*lhs) == &(*rhs));
}

void test_reference_extension()
{
#if defined(_LIBCPP_VERSION) && 0 // FIXME these extensions are currently disabled.
    using T = TestTypes::TestType;
    T::reset();
    {
        T t;
        T::reset_constructors();
        test_ref<T&>();
        test_ref<T&>(t);
        assert(T::alive == 1);
        assert(T::constructed == 0);
        assert(T::assigned == 0);
        assert(T::destroyed == 0);
    }
    assert(T::destroyed == 1);
    assert(T::alive == 0);
    {
        T t;
        const T& ct = t;
        T::reset_constructors();
        test_ref<T const&>();
        test_ref<T const&>(t);
        test_ref<T const&>(ct);
        assert(T::alive == 1);
        assert(T::constructed == 0);
        assert(T::assigned == 0);
        assert(T::destroyed == 0);
    }
    assert(T::alive == 0);
    assert(T::destroyed == 1);
    {
        T t;
        T::reset_constructors();
        test_ref<T&&>();
        test_ref<T&&>(std::move(t));
        assert(T::alive == 1);
        assert(T::constructed == 0);
        assert(T::assigned == 0);
        assert(T::destroyed == 0);
    }
    assert(T::alive == 0);
    assert(T::destroyed == 1);
    {
        T t;
        const T& ct = t;
        T::reset_constructors();
        test_ref<T const&&>();
        test_ref<T const&&>(std::move(t));
        test_ref<T const&&>(std::move(ct));
        assert(T::alive == 1);
        assert(T::constructed == 0);
        assert(T::assigned == 0);
        assert(T::destroyed == 0);
    }
    assert(T::alive == 0);
    assert(T::destroyed == 1);
    {
        static_assert(!std::is_copy_constructible<std::optional<T&&>>::value, "");
        static_assert(!std::is_copy_constructible<std::optional<T const&&>>::value, "");
    }
#endif
}


int main()
{
    test<int>();
    test<int>(3);
    static_assert(constexpr_test<int>(), "" );
    static_assert(constexpr_test<int>(3), "" );

    {
        optional<const int> o(42);
        optional<const int> o2(std::move(o));
        assert(*o2 == 42);
    }
    {
        using T = TestTypes::TestType;
        T::reset();
        optional<T> rhs;
        assert(T::alive == 0);
        const optional<T> lhs(std::move(rhs));
        assert(lhs.has_value() == false);
        assert(rhs.has_value() == false);
        assert(T::alive == 0);
    }
    TestTypes::TestType::reset();
    {
        using T = TestTypes::TestType;
        T::reset();
        optional<T> rhs(42);
        assert(T::alive == 1);
        assert(T::value_constructed == 1);
        assert(T::move_constructed == 0);
        const optional<T> lhs(std::move(rhs));
        assert(lhs.has_value());
        assert(rhs.has_value());
        assert(lhs.value().value == 42);
        assert(rhs.value().value == -1);
        assert(T::move_constructed == 1);
        assert(T::alive == 2);
    }
    TestTypes::TestType::reset();
    {
        using namespace ConstexprTestTypes;
        test<TestType>();
        test<TestType>(42);
    }
    {
        using namespace TrivialTestTypes;
        test<TestType>();
        test<TestType>(42);
    }
    {
        test_throwing_ctor();
    }
    {
        struct ThrowsMove {
          ThrowsMove() noexcept(false) {}
          ThrowsMove(ThrowsMove const&) noexcept(false) {}
          ThrowsMove(ThrowsMove &&) noexcept(false) {}
        };
        static_assert(!std::is_nothrow_move_constructible<optional<ThrowsMove>>::value, "");
        struct NoThrowMove {
          NoThrowMove() noexcept(false) {}
          NoThrowMove(NoThrowMove const&) noexcept(false) {}
          NoThrowMove(NoThrowMove &&) noexcept(true) {}
        };
        static_assert(std::is_nothrow_move_constructible<optional<NoThrowMove>>::value, "");
    }
    {
        test_reference_extension();
    }
    {
    constexpr std::optional<int> o1{4};
    constexpr std::optional<int> o2 = std::move(o1);
    static_assert( *o2 == 4, "" );
    }
}
