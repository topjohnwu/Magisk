//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <utility>

// template <class T1, class T2> struct pair

// template<class U, class V> pair& operator=(tuple<U, V>&& p);

#include <utility>
#include <tuple>
#include <array>
#include <memory>
#include <cassert>

#include "archetypes.hpp"

// Clang warns about missing braces when initializing std::array.
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wmissing-braces"
#endif

int main()
{
    using C = TestTypes::TestType;
    {
       using P = std::pair<int, C>;
       using T = std::tuple<int, C>;
       T t(42, C{42});
       P p(101, C{101});
       C::reset_constructors();
       p = t;
       assert(C::constructed == 0);
       assert(C::assigned == 1);
       assert(C::copy_assigned == 1);
       assert(C::move_assigned == 0);
       assert(p.first == 42);
       assert(p.second.value == 42);
    }
    {
       using P = std::pair<int, C>;
       using T = std::tuple<int, C>;
       T t(42, -42);
       P p(101, 101);
       C::reset_constructors();
       p = std::move(t);
       assert(C::constructed == 0);
       assert(C::assigned == 1);
       assert(C::copy_assigned == 0);
       assert(C::move_assigned == 1);
       assert(p.first == 42);
       assert(p.second.value == -42);
    }
    {
       using P = std::pair<C, C>;
       using T = std::array<C, 2>;
       T t = {42, -42};
       P p{101, 101};
       C::reset_constructors();
       p = t;
       assert(C::constructed == 0);
       assert(C::assigned == 2);
       assert(C::copy_assigned == 2);
       assert(C::move_assigned == 0);
       assert(p.first.value == 42);
       assert(p.second.value == -42);
    }
    {
       using P = std::pair<C, C>;
       using T = std::array<C, 2>;
       T t = {42, -42};
       P p{101, 101};
       C::reset_constructors();
       p = t;
       assert(C::constructed == 0);
       assert(C::assigned == 2);
       assert(C::copy_assigned == 2);
       assert(C::move_assigned == 0);
       assert(p.first.value == 42);
       assert(p.second.value == -42);
    }
    {
       using P = std::pair<C, C>;
       using T = std::array<C, 2>;
       T t = {42, -42};
       P p{101, 101};
       C::reset_constructors();
       p = std::move(t);
       assert(C::constructed == 0);
       assert(C::assigned == 2);
       assert(C::copy_assigned == 0);
       assert(C::move_assigned == 2);
       assert(p.first.value == 42);
       assert(p.second.value == -42);
    }
}
