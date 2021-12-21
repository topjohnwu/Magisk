//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <tuple>

// template <class... Types> class tuple;

// template <class Alloc>
//   tuple(allocator_arg_t, const Alloc& a);

// NOTE: this constructor does not currently support tags derived from
// allocator_arg_t because libc++ has to deduce the parameter as a template
// argument. See PR27684 (https://bugs.llvm.org/show_bug.cgi?id=27684)

#include <tuple>
#include <cassert>

#include "DefaultOnly.h"
#include "allocators.h"
#include "../alloc_first.h"
#include "../alloc_last.h"

template <class T = void>
struct NonDefaultConstructible {
  constexpr NonDefaultConstructible() {
      static_assert(!std::is_same<T, T>::value, "Default Ctor instantiated");
  }

  explicit constexpr NonDefaultConstructible(int) {}
};


struct DerivedFromAllocArgT : std::allocator_arg_t {};

int main()
{
    {
        std::tuple<> t(std::allocator_arg, A1<int>());
    }
    {
        std::tuple<int> t(std::allocator_arg, A1<int>());
        assert(std::get<0>(t) == 0);
    }
    {
        std::tuple<DefaultOnly> t(std::allocator_arg, A1<int>());
        assert(std::get<0>(t) == DefaultOnly());
    }
    {
        assert(!alloc_first::allocator_constructed);
        std::tuple<alloc_first> t(std::allocator_arg, A1<int>(5));
        assert(alloc_first::allocator_constructed);
        assert(std::get<0>(t) == alloc_first());
    }
    {
        assert(!alloc_last::allocator_constructed);
        std::tuple<alloc_last> t(std::allocator_arg, A1<int>(5));
        assert(alloc_last::allocator_constructed);
        assert(std::get<0>(t) == alloc_last());
    }
    {
        alloc_first::allocator_constructed = false;
        std::tuple<DefaultOnly, alloc_first> t(std::allocator_arg, A1<int>(5));
        assert(std::get<0>(t) == DefaultOnly());
        assert(alloc_first::allocator_constructed);
        assert(std::get<1>(t) == alloc_first());
    }
    {
        alloc_first::allocator_constructed = false;
        alloc_last::allocator_constructed = false;
        std::tuple<DefaultOnly, alloc_first, alloc_last> t(std::allocator_arg,
                                                           A1<int>(5));
        assert(std::get<0>(t) == DefaultOnly());
        assert(alloc_first::allocator_constructed);
        assert(std::get<1>(t) == alloc_first());
        assert(alloc_last::allocator_constructed);
        assert(std::get<2>(t) == alloc_last());
    }
    {
        alloc_first::allocator_constructed = false;
        alloc_last::allocator_constructed = false;
        std::tuple<DefaultOnly, alloc_first, alloc_last> t(std::allocator_arg,
                                                           A2<int>(5));
        assert(std::get<0>(t) == DefaultOnly());
        assert(!alloc_first::allocator_constructed);
        assert(std::get<1>(t) == alloc_first());
        assert(!alloc_last::allocator_constructed);
        assert(std::get<2>(t) == alloc_last());
    }
    {
        // Test that the uses-allocator default constructor does not evaluate
        // its SFINAE when it otherwise shouldn't be selected. Do this by
        // using 'NonDefaultConstructible' which will cause a compile error
        // if std::is_default_constructible is evaluated on it.
        using T = NonDefaultConstructible<>;
        T v(42);
        std::tuple<T, T> t(v, v);
        (void)t;
        std::tuple<T, T> t2(42, 42);
        (void)t2;
    }
}
