//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03, c++11, c++14, c++17

// <map>

//   template <class Key, class T, class Compare, class Allocator, class Predicate>
//     void erase_if(multimap<Key, T, Compare, Allocator>& c, Predicate pred);

#include <map>

#include "test_macros.h"
#include "test_allocator.h"
#include "min_allocator.h"

using Init = std::initializer_list<int>;
template <typename M>
M make (Init vals)
{
    M ret;
    for (int v : vals)
        ret.insert(typename M::value_type(v, v + 10));
    return ret;
}

template <typename M, typename Pred>
void
test0(Init vals, Pred p, Init expected)
{
    M s = make<M> (vals);
    ASSERT_SAME_TYPE(void, decltype(std::erase_if(s, p)));
    std::erase_if(s, p);
    assert(s == make<M>(expected));
}

template <typename S>
void test()
{
    auto is1 = [](auto v) { return v.first == 1;};
    auto is2 = [](auto v) { return v.first == 2;};
    auto is3 = [](auto v) { return v.first == 3;};
    auto is4 = [](auto v) { return v.first == 4;};
    auto True  = [](auto) { return true; };
    auto False = [](auto) { return false; };
    
    test0<S>({}, is1, {});

    test0<S>({1}, is1, {});
    test0<S>({1}, is2, {1});

    test0<S>({1,2}, is1, {2});
    test0<S>({1,2}, is2, {1});
    test0<S>({1,2}, is3, {1,2});
    test0<S>({1,1}, is1, {});
    test0<S>({1,1}, is3, {1,1});

    test0<S>({1,2,3}, is1, {2,3});
    test0<S>({1,2,3}, is2, {1,3});
    test0<S>({1,2,3}, is3, {1,2});
    test0<S>({1,2,3}, is4, {1,2,3});

    test0<S>({1,1,1}, is1, {});
    test0<S>({1,1,1}, is2, {1,1,1});
    test0<S>({1,1,2}, is1, {2});
    test0<S>({1,1,2}, is2, {1,1});
    test0<S>({1,1,2}, is3, {1,1,2});
    test0<S>({1,2,2}, is1, {2,2});
    test0<S>({1,2,2}, is2, {1});
    test0<S>({1,2,2}, is3, {1,2,2});
    
    test0<S>({1,2,3}, True,  {});
    test0<S>({1,2,3}, False, {1,2,3});
}

int main()
{
    test<std::multimap<int, int>>();
    test<std::multimap<int, int, std::less<int>, min_allocator<std::pair<const int, int>>>> ();
    test<std::multimap<int, int, std::less<int>, test_allocator<std::pair<const int, int>>>> ();

    test<std::multimap<long, short>>();
    test<std::multimap<short, double>>();
}
