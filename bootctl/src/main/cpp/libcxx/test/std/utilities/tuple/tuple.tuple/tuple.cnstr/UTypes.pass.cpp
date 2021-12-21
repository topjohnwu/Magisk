//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <tuple>

// template <class... Types> class tuple;

// template <class... UTypes>
//   explicit tuple(UTypes&&... u);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <cassert>
#include <type_traits>

#include "test_macros.h"
#include "test_convertible.hpp"
#include "MoveOnly.h"

#if TEST_STD_VER > 11

struct Empty {};
struct A
{
    int id_;
    explicit constexpr A(int i) : id_(i) {}
};

#endif

struct NoDefault { NoDefault() = delete; };

// Make sure the _Up... constructor SFINAEs out when the types that
// are not explicitly initialized are not all default constructible.
// Otherwise, std::is_constructible would return true but instantiating
// the constructor would fail.
void test_default_constructible_extension_sfinae()
{
    {
        typedef std::tuple<MoveOnly, NoDefault> Tuple;

        static_assert(!std::is_constructible<
            Tuple,
            MoveOnly
        >::value, "");

        static_assert(std::is_constructible<
            Tuple,
            MoveOnly, NoDefault
        >::value, "");
    }
    {
        typedef std::tuple<MoveOnly, MoveOnly, NoDefault> Tuple;

        static_assert(!std::is_constructible<
            Tuple,
            MoveOnly, MoveOnly
        >::value, "");

        static_assert(std::is_constructible<
            Tuple,
            MoveOnly, MoveOnly, NoDefault
        >::value, "");
    }
    {
        // Same idea as above but with a nested tuple type.
        typedef std::tuple<MoveOnly, NoDefault> Tuple;
        typedef std::tuple<MoveOnly, Tuple, MoveOnly, MoveOnly> NestedTuple;

        static_assert(!std::is_constructible<
            NestedTuple,
            MoveOnly, MoveOnly, MoveOnly, MoveOnly
        >::value, "");

        static_assert(std::is_constructible<
            NestedTuple,
            MoveOnly, Tuple, MoveOnly, MoveOnly
        >::value, "");
    }
    // testing extensions
#ifdef _LIBCPP_VERSION
    {
        typedef std::tuple<MoveOnly, int> Tuple;
        typedef std::tuple<MoveOnly, Tuple, MoveOnly, MoveOnly> NestedTuple;

        static_assert(std::is_constructible<
            NestedTuple,
            MoveOnly, MoveOnly, MoveOnly, MoveOnly
        >::value, "");

        static_assert(std::is_constructible<
            NestedTuple,
            MoveOnly, Tuple, MoveOnly, MoveOnly
        >::value, "");
    }
#endif
}

int main()
{
    {
        std::tuple<MoveOnly> t(MoveOnly(0));
        assert(std::get<0>(t) == 0);
    }
    {
        std::tuple<MoveOnly, MoveOnly> t(MoveOnly(0), MoveOnly(1));
        assert(std::get<0>(t) == 0);
        assert(std::get<1>(t) == 1);
    }
    {
        std::tuple<MoveOnly, MoveOnly, MoveOnly> t(MoveOnly(0),
                                                   MoveOnly(1),
                                                   MoveOnly(2));
        assert(std::get<0>(t) == 0);
        assert(std::get<1>(t) == 1);
        assert(std::get<2>(t) == 2);
    }
    // extensions
#ifdef _LIBCPP_VERSION
    {
        using E = MoveOnly;
        using Tup = std::tuple<E, E, E>;
        // Test that the reduced arity initialization extension is only
        // allowed on the explicit constructor.
        static_assert(test_convertible<Tup, E, E, E>(), "");

        Tup t(E(0), E(1));
        static_assert(!test_convertible<Tup, E, E>(), "");
        assert(std::get<0>(t) == 0);
        assert(std::get<1>(t) == 1);
        assert(std::get<2>(t) == MoveOnly());

        Tup t2(E(0));
        static_assert(!test_convertible<Tup, E>(), "");
        assert(std::get<0>(t) == 0);
        assert(std::get<1>(t) == E());
        assert(std::get<2>(t) == E());
    }
#endif
#if TEST_STD_VER > 11
    {
        constexpr std::tuple<Empty> t0{Empty()};
        (void)t0;
    }
    {
        constexpr std::tuple<A, A> t(3, 2);
        static_assert(std::get<0>(t).id_ == 3, "");
    }
#endif
    // Check that SFINAE is properly applied with the default reduced arity
    // constructor extensions.
    test_default_constructible_extension_sfinae();
}
