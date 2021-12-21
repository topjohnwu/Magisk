//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <stack>

// template <class T, class Container = deque<T>>
// class stack
// {
// public:
//     typedef Container                                container_type;
//     typedef typename container_type::value_type      value_type;
//     typedef typename container_type::reference       reference;
//     typedef typename container_type::const_reference const_reference;
//     typedef typename container_type::size_type       size_type;
//
// protected:
//     container_type c;
// ...
// };

#include <stack>
#include <vector>
#include <type_traits>

struct test
    : private std::stack<int>
{
    test()
    {
        c.push_back(1);
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
    static_assert(( std::is_same<std::stack<int>::container_type, std::deque<int> >::value), "");
    static_assert(( std::is_same<std::stack<int, std::vector<int> >::container_type, std::vector<int> >::value), "");
    static_assert(( std::is_same<std::stack<int, std::vector<int> >::value_type, int>::value), "");
    static_assert(( std::is_same<std::stack<int>::reference, std::deque<int>::reference>::value), "");
    static_assert(( std::is_same<std::stack<int>::const_reference, std::deque<int>::const_reference>::value), "");
    static_assert(( std::is_same<std::stack<int>::size_type, std::deque<int>::size_type>::value), "");
    static_assert(( std::uses_allocator<std::stack<int>, std::allocator<int> >::value), "");
    static_assert((!std::uses_allocator<std::stack<int, C>, std::allocator<int> >::value), "");
    test t;
}
