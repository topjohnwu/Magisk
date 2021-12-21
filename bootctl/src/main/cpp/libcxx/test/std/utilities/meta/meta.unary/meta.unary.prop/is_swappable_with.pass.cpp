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

// is_swappable_with

#include <type_traits>
#include <vector>
#include "test_macros.h"

namespace MyNS {

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

struct M {};

void swap(M&&, M&&) {}

} // namespace MyNS

int main()
{
    using namespace MyNS;
    {
        // Test that is_swappable_with doesn't apply an lvalue reference
        // to the type. Instead it is up to the user.
        static_assert(!std::is_swappable_with<int, int>::value, "");
        static_assert(std::is_swappable_with<int&, int&>::value, "");
        static_assert(std::is_swappable_with<M, M>::value, "");
        static_assert(std::is_swappable_with<A&, A&>::value, "");
    }
    {
        // test that heterogeneous swap is allowed only if both 'swap(A, B)' and
        // 'swap(B, A)' are valid.
        static_assert(std::is_swappable_with<A&, B&>::value, "");
        static_assert(!std::is_swappable_with<A&, C&>::value, "");
        static_assert(!std::is_swappable_with<D&, C&>::value, "");
    }
    {
        // test that cv void is guarded against as required.
        static_assert(!std::is_swappable_with_v<void, int>, "");
        static_assert(!std::is_swappable_with_v<int, void>, "");
        static_assert(!std::is_swappable_with_v<const void, const volatile void>, "");
    }
    {
        // test for presence of is_swappable_with_v
        static_assert(std::is_swappable_with_v<int&, int&>, "");
        static_assert(!std::is_swappable_with_v<D&, C&>, "");
    }
}
