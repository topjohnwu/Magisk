//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// type_traits

// is_swappable

#include <type_traits>
#include <utility>
#include <vector>
#include "test_macros.h"

namespace MyNS {

// Make the test types non-copyable so that generic std::swap is not valid.
struct A {
  A(A const&) = delete;
  A& operator=(A const&) = delete;
};

struct B {
  B(B const&) = delete;
  B& operator=(B const&) = delete;
};

struct C {};
struct D {};

void swap(A&, A&) {}

void swap(A&, B&) {}
void swap(B&, A&) {}

void swap(A&, C&) {} // missing swap(C, A)
void swap(D&, C&) {}

struct M {
  M(M const&) = delete;
  M& operator=(M const&) = delete;
};

void swap(M&&, M&&) {}

struct DeletedSwap {
  friend void swap(DeletedSwap&, DeletedSwap&) = delete;
};

} // namespace MyNS

namespace MyNS2 {

struct AmbiguousSwap {};

template <class T>
void swap(T&, T&) {}

} // end namespace MyNS2

int main()
{
    using namespace MyNS;
    {
        // Test that is_swappable applies an lvalue reference to the type.
        static_assert(std::is_swappable<A>::value, "");
        static_assert(std::is_swappable<A&>::value, "");
        static_assert(!std::is_swappable<M>::value, "");
        static_assert(!std::is_swappable<M&&>::value, "");
    }
    static_assert(!std::is_swappable<B>::value, "");
    static_assert(std::is_swappable<C>::value, "");
    {
        // test non-referencable types
        static_assert(!std::is_swappable<void>::value, "");
        static_assert(!std::is_swappable<int() const>::value, "");
        static_assert(!std::is_swappable<int() &>::value, "");
    }
    {
        // test that a deleted swap is correctly handled.
        static_assert(!std::is_swappable<DeletedSwap>::value, "");
    }
    {
        // test that a swap with ambiguous overloads is handled correctly.
        static_assert(!std::is_swappable<MyNS2::AmbiguousSwap>::value, "");
    }
    {
        // test for presence of is_swappable_v
        static_assert(std::is_swappable_v<int>, "");
        static_assert(!std::is_swappable_v<M>, "");
    }
}
