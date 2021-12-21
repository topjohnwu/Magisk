//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <deque>

// iterator erase(const_iterator f)

//  Erasing items from the beginning or the end of a deque shall not invalidate iterators
//  to items that were not erased.

#include <deque>
#include <cassert>

template <typename C>
void del_at_start(C c)
{
    typename C::iterator first = c.begin();
    typename C::iterator it1 = first + 1;
    typename C::iterator it2 = c.end() - 1;

    c.erase (first);

    typename C::iterator it3 = c.begin();
    typename C::iterator it4 = c.end() - 1;
    assert(  it1 ==   it3);
    assert( *it1 ==  *it3);
    assert(&*it1 == &*it3);
    assert(  it2 ==   it4);
    assert( *it2 ==  *it4);
    assert(&*it2 == &*it4);
}

template <typename C>
void del_at_end(C c)
{
    typename C::iterator first = c.end() - 1;
    typename C::iterator it1 = c.begin();
    typename C::iterator it2 = first - 1;

    c.erase (first);

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
        del_at_start(queue);
        del_at_end(queue);
        queue.pop_back();
    }
}
