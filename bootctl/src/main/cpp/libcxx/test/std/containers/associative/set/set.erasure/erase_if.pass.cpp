//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <set>

// template <class T, class Compare, class Allocator, class Predicate>
//   void erase_if(set<T, Compare, Allocator>& c, Predicate pred);

#include <set>

#include "test_macros.h"
#include "test_allocator.h"
#include "min_allocator.h"

template <class S, class Pred>
void
test0(S s, Pred p, S expected)
{
    ASSERT_SAME_TYPE(void, decltype(std::erase_if(s, p)));
    std::erase_if(s, p);
    assert(s == expected);
}

template <typename S>
void test()
{
    auto is1 = [](auto v) { return v == 1;};
    auto is2 = [](auto v) { return v == 2;};
    auto is3 = [](auto v) { return v == 3;};
    auto is4 = [](auto v) { return v == 4;};
    auto True  = [](auto) { return true; };
    auto False = [](auto) { return false; };
    
    test0(S(), is1, S());

    test0(S({1}), is1, S());
    test0(S({1}), is2, S({1}));

    test0(S({1,2}), is1, S({2}));
    test0(S({1,2}), is2, S({1}));
    test0(S({1,2}), is3, S({1,2}));

    test0(S({1,2,3}), is1, S({2,3}));
    test0(S({1,2,3}), is2, S({1,3}));
    test0(S({1,2,3}), is3, S({1,2}));
    test0(S({1,2,3}), is4, S({1,2,3}));

    test0(S({1,2,3}), True,  S());
    test0(S({1,2,3}), False, S({1,2,3}));
}

int main()
{
    test<std::set<int>>();
    test<std::set<int, std::less<int>, min_allocator<int>>> ();
    test<std::set<int, std::less<int>, test_allocator<int>>> ();

    test<std::set<long>>();
    test<std::set<double>>();
}
