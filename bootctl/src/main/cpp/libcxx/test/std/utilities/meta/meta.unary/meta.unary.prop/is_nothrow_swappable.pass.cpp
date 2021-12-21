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

void swap(A&, A&) noexcept {}
void swap(B&, B&) {}

struct M {
  M(M const&) = delete;
  M& operator=(M const&) = delete;
};

void swap(M&&, M&&) noexcept {}

struct ThrowingMove {
    ThrowingMove(ThrowingMove&&) {}
    ThrowingMove& operator=(ThrowingMove&&) { return *this; }
};

} // namespace MyNS

int main()
{
    using namespace MyNS;
    {
        // Test that is_swappable applies an lvalue reference to the type.
        static_assert(std::is_nothrow_swappable<int>::value, "");
        static_assert(std::is_nothrow_swappable<int&>::value, "");
        static_assert(!std::is_nothrow_swappable<M>::value, "");
        static_assert(!std::is_nothrow_swappable<M&&>::value, "");
    }
    {
        // Test that it correctly deduces the noexcept of swap.
        static_assert(std::is_nothrow_swappable<A>::value, "");
        static_assert(!std::is_nothrow_swappable<B>::value
                      && std::is_swappable<B>::value, "");
        static_assert(!std::is_nothrow_swappable<ThrowingMove>::value
                      && std::is_swappable<ThrowingMove>::value, "");
    }
    {
        // Test that it doesn't drop the qualifiers
        static_assert(!std::is_nothrow_swappable<const A>::value, "");
    }
    {
        // test non-referenceable types
        static_assert(!std::is_nothrow_swappable<void>::value, "");
        static_assert(!std::is_nothrow_swappable<int() const>::value, "");
        static_assert(!std::is_nothrow_swappable<int(int, ...) const &>::value, "");
    }
    {
        // test for presence of is_nothrow_swappable_v
        static_assert(std::is_nothrow_swappable_v<int>, "");
        static_assert(!std::is_nothrow_swappable_v<void>, "");
    }
}
