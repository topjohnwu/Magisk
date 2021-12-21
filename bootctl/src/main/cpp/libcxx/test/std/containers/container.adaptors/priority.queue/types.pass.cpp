//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <queue>

// template <class T, class Container = vector<T>,
//           class Compare = less<typename Container::value_type>>
// class priority_queue
// {
// public:
//     typedef Container                                container_type;
//     typedef Compare                                  value_compare; // LWG#2684
//     typedef typename container_type::value_type      value_type;
//     typedef typename container_type::reference       reference;
//     typedef typename container_type::const_reference const_reference;
//     typedef typename container_type::size_type       size_type;
//
// protected:
//     container_type c;
//     Compare comp;

#include <queue>
#include <cassert>
#include <deque>
#include <functional>
#include <memory>
#include <type_traits>
#include <vector>

struct test
    : private std::priority_queue<int>
{
    test()
    {
        c.push_back(1);
        assert(comp(1, 2));
    }
};

struct C
{
    typedef int value_type;
    typedef int& reference;
    typedef const int& const_reference;
    typedef int size_type;
};

int main()
{
    static_assert(( std::is_same<std::priority_queue<int>::container_type, std::vector<int> >::value), "");
    static_assert(( std::is_same<std::priority_queue<int, std::deque<int> >::container_type, std::deque<int> >::value), "");
    static_assert(( std::is_same<std::priority_queue<int, std::deque<int> >::value_type, int>::value), "");
    static_assert(( std::is_same<std::priority_queue<int>::reference, std::vector<int>::reference>::value), "");
    static_assert(( std::is_same<std::priority_queue<int>::const_reference, std::vector<int>::const_reference>::value), "");
    static_assert(( std::is_same<std::priority_queue<int>::size_type, std::vector<int>::size_type>::value), "");
    static_assert(( std::is_same<std::priority_queue<int>::value_compare, std::less<int> >::value), "");
    static_assert(( std::is_same<std::priority_queue<int, std::deque<int> >::value_compare, std::less<int> >::value), "");
    static_assert(( std::is_same<std::priority_queue<int, std::deque<int>, std::greater<int> >::value_compare, std::greater<int> >::value), "");
    static_assert(( std::uses_allocator<std::priority_queue<int>, std::allocator<int> >::value), "");
    static_assert((!std::uses_allocator<std::priority_queue<int, C>, std::allocator<int> >::value), "");
    test t;
}
