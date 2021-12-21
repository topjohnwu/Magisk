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

// template <class Alloc, class... UTypes>
//   tuple(allocator_arg_t, const Alloc& a, UTypes&&...);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <cassert>

#include "MoveOnly.h"
#include "allocators.h"
#include "../alloc_first.h"
#include "../alloc_last.h"

template <class T = void>
struct DefaultCtorBlowsUp {
  constexpr DefaultCtorBlowsUp() {
      static_assert(!std::is_same<T, T>::value, "Default Ctor instantiated");
  }

  explicit constexpr DefaultCtorBlowsUp(int x) : value(x) {}

  int value;
};


struct DerivedFromAllocArgT : std::allocator_arg_t {};


// Make sure the _Up... constructor SFINAEs out when the number of initializers
// is less that the number of elements in the tuple. Previously libc++ would
// offer these constructers as an extension but they broke conforming code.
void test_uses_allocator_sfinae_evaluation()
{
     using BadDefault = DefaultCtorBlowsUp<>;
    {
        typedef std::tuple<MoveOnly, MoveOnly, BadDefault> Tuple;

        static_assert(!std::is_constructible<
            Tuple,
            std::allocator_arg_t, A1<int>, MoveOnly
        >::value, "");

        static_assert(std::is_constructible<
            Tuple,
            std::allocator_arg_t, A1<int>, MoveOnly, MoveOnly, BadDefault
        >::value, "");
    }
    {
        typedef std::tuple<MoveOnly, MoveOnly, BadDefault, BadDefault> Tuple;

        static_assert(!std::is_constructible<
            Tuple,
            std::allocator_arg_t, A1<int>, MoveOnly, MoveOnly
        >::value, "");

        static_assert(std::is_constructible<
            Tuple,
            std::allocator_arg_t, A1<int>, MoveOnly, MoveOnly, BadDefault, BadDefault
        >::value, "");
    }
}

struct Explicit {
  int value;
  explicit Explicit(int x) : value(x) {}
};

int main()
{
    {
        std::tuple<Explicit> t{std::allocator_arg, std::allocator<void>{}, 42};
        assert(std::get<0>(t).value == 42);
    }
    {
        std::tuple<MoveOnly> t(std::allocator_arg, A1<int>(), MoveOnly(0));
        assert(std::get<0>(t) == 0);
    }
    {
        using T = DefaultCtorBlowsUp<>;
        std::tuple<T> t(std::allocator_arg, A1<int>(), T(42));
        assert(std::get<0>(t).value == 42);
    }
    {
        std::tuple<MoveOnly, MoveOnly> t(std::allocator_arg, A1<int>(),
                                         MoveOnly(0), MoveOnly(1));
        assert(std::get<0>(t) == 0);
        assert(std::get<1>(t) == 1);
    }
    {
        using T = DefaultCtorBlowsUp<>;
        std::tuple<T, T> t(std::allocator_arg, A1<int>(), T(42), T(43));
        assert(std::get<0>(t).value == 42);
        assert(std::get<1>(t).value == 43);
    }
    {
        std::tuple<MoveOnly, MoveOnly, MoveOnly> t(std::allocator_arg, A1<int>(),
                                                   MoveOnly(0),
                                                   1, 2);
        assert(std::get<0>(t) == 0);
        assert(std::get<1>(t) == 1);
        assert(std::get<2>(t) == 2);
    }
    {
        using T = DefaultCtorBlowsUp<>;
        std::tuple<T, T, T> t(std::allocator_arg, A1<int>(), T(1), T(2), T(3));
        assert(std::get<0>(t).value == 1);
        assert(std::get<1>(t).value == 2);
        assert(std::get<2>(t).value == 3);
    }
    {
        alloc_first::allocator_constructed = false;
        alloc_last::allocator_constructed = false;
        std::tuple<int, alloc_first, alloc_last> t(std::allocator_arg,
                                                   A1<int>(5), 1, 2, 3);
        assert(std::get<0>(t) == 1);
        assert(alloc_first::allocator_constructed);
        assert(std::get<1>(t) == alloc_first(2));
        assert(alloc_last::allocator_constructed);
        assert(std::get<2>(t) == alloc_last(3));
    }
    {
        // Check that uses-allocator construction is still selected when
        // given a tag type that derives from allocator_arg_t.
        DerivedFromAllocArgT tag;
        alloc_first::allocator_constructed = false;
        alloc_last::allocator_constructed = false;
        std::tuple<int, alloc_first, alloc_last> t(tag,
                                                   A1<int>(5), 1, 2, 3);
        assert(std::get<0>(t) == 1);
        assert(alloc_first::allocator_constructed);
        assert(std::get<1>(t) == alloc_first(2));
        assert(alloc_last::allocator_constructed);
        assert(std::get<2>(t) == alloc_last(3));
    }
    // Stress test the SFINAE on the uses-allocator constructors and
    // ensure that the "reduced-arity-initialization" extension is not offered
    // for these constructors.
    test_uses_allocator_sfinae_evaluation();
}
