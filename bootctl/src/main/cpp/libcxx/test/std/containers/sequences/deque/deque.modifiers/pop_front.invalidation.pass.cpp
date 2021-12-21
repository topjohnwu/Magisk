//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <deque>

// void pop_front()

//  Erasing items from the beginning or the end of a deque shall not invalidate iterators
//  to items that were not erased.

#include <deque>
#include <cassert>

template <typename C>
void test(C c)
{
    typename C::iterator it1 = c.begin() + 1;
    typename C::iterator it2 = c.end() - 1;

    c.pop_front();

    typename C::iterator it3 = c.begin();
    typename C::iterator it4 = c.end() - 1;
    assert(  it1 ==   it3);
    assert( *it1 ==  *it3);
    assert(&*it1 == &*it3);
    assert(  it2 ==   it4);
    assert( *it2 ==  *it4);
    assert(&*it2 == &*it4);
}

int main()
{
    std::deque<int> queue;
    for (int i = 0; i < 20; ++i)
        queue.push_back(i);

    while (queue.size() > 1)
    {
        test(queue);
        queue.pop_back();
    }
}
