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

// MODULES_DEFINES: _LIBCPP_ENABLE_TUPLE_IMPLICIT_REDUCED_ARITY_EXTENSION
#define _LIBCPP_ENABLE_TUPLE_IMPLICIT_REDUCED_ARITY_EXTENSION
#include <tuple>
#include <cassert>
#include <type_traits>
#include <string>
#include <system_error>

#include "test_macros.h"
#include "test_convertible.hpp"
#include "MoveOnly.h"


struct NoDefault { NoDefault() = delete; };


// Make sure the _Up... constructor SFINAEs out when the types that
// are not explicitly initialized are not all default constructible.
// Otherwise, std::is_constructible would return true but instantiating
// the constructor would fail.
void test_default_constructible_extension_sfinae()
{
    typedef MoveOnly MO;
    typedef NoDefault ND;
    {
        typedef std::tuple<MO, ND> Tuple;
        static_assert(!std::is_constructible<Tuple, MO>::value, "");
        static_assert(std::is_constructible<Tuple, MO, ND>::value, "");
        static_assert(test_convertible<Tuple, MO, ND>(), "");
    }
    {
        typedef std::tuple<MO, MO, ND> Tuple;
        static_assert(!std::is_constructible<Tuple, MO, MO>::value, "");
        static_assert(std::is_constructible<Tuple, MO, MO, ND>::value, "");
        static_assert(test_convertible<Tuple, MO, MO, ND>(), "");
    }
    {
        // Same idea as above but with a nested tuple type.
        typedef std::tuple<MO, ND> Tuple;
        typedef std::tuple<MO, Tuple, MO, MO> NestedTuple;

        static_assert(!std::is_constructible<
            NestedTuple, MO, MO, MO, MO>::value, "");
        static_assert(std::is_constructible<
            NestedTuple, MO, Tuple, MO, MO>::value, "");
    }
    {
        typedef std::tuple<MO, int> Tuple;
        typedef std::tuple<MO, Tuple, MO, MO> NestedTuple;

        static_assert(std::is_constructible<
            NestedTuple, MO, MO, MO, MO>::value, "");
        static_assert(test_convertible<
            NestedTuple, MO, MO, MO, MO>(), "");

        static_assert(std::is_constructible<
            NestedTuple, MO, Tuple, MO, MO>::value, "");
        static_assert(test_convertible<
            NestedTuple, MO, Tuple, MO, MO>(), "");
    }
}

std::tuple<std::string, int, std::error_code> doc_example() {
      return {"hello world", 42};
}

// Test that the example given in UsingLibcxx.rst actually works.
void test_example_from_docs() {
  auto tup = doc_example();
  assert(std::get<0>(tup) == "hello world");
  assert(std::get<1>(tup) == 42);
  assert(std::get<2>(tup) == std::error_code{});
}

int main()
{

    {
        using E = MoveOnly;
        using Tup = std::tuple<E, E, E>;
        static_assert(test_convertible<Tup, E, E, E>(), "");

        Tup t = {E(0), E(1)};
        static_assert(test_convertible<Tup, E, E>(), "");
        assert(std::get<0>(t) == 0);
        assert(std::get<1>(t) == 1);
        assert(std::get<2>(t) == MoveOnly());

        Tup t2 = {E(0)};
        static_assert(test_convertible<Tup, E>(), "");
        assert(std::get<0>(t) == 0);
        assert(std::get<1>(t) == E());
        assert(std::get<2>(t) == E());
    }
    // Check that SFINAE is properly applied with the default reduced arity
    // constructor extensions.
    test_default_constructible_extension_sfinae();
    test_example_from_docs();
}
