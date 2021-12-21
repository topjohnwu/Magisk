//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <iterator>

// insert_iterator

// requires CopyConstructible<Cont::value_type>
//   insert_iterator<Cont>&
//   operator=(const Cont::value_type& value);

#include <iterator>

#include <utility>
#include <vector>
#include <memory>
#include <cassert>

template <class C>
void
test(C c1, typename C::difference_type j,
     typename C::value_type x1, typename C::value_type x2,
     typename C::value_type x3, const C& c2)
{
    std::insert_iterator<C> q(c1, c1.begin() + j);
    q = std::move(x1);
    q = std::move(x2);
    q = std::move(x3);
    assert(c1 == c2);
}

template <class C>
void
insert3at(C& c, typename C::iterator i,
     typename C::value_type x1, typename C::value_type x2,
     typename C::value_type x3)
{
    i = c.insert(i, std::move(x1));
    i = c.insert(++i, std::move(x2));
    c.insert(++i, std::move(x3));
}

struct do_nothing
{
    void operator()(void*) const {}
};

int main()
{
    {
    typedef std::unique_ptr<int, do_nothing> Ptr;
    typedef std::vector<Ptr> C;
    C c1;
    int x[6] = {0};
    for (int i = 0; i < 3; ++i)
        c1.push_back(Ptr(x+i));
    C c2;
    for (int i = 0; i < 3; ++i)
        c2.push_back(Ptr(x+i));
    insert3at(c2, c2.begin(), Ptr(x+3), Ptr(x+4), Ptr(x+5));
    test(std::move(c1), 0, Ptr(x+3), Ptr(x+4), Ptr(x+5), c2);
    c1.clear();
    for (int i = 0; i < 3; ++i)
        c1.push_back(Ptr(x+i));
    c2.clear();
    for (int i = 0; i < 3; ++i)
        c2.push_back(Ptr(x+i));
    insert3at(c2, c2.begin()+1, Ptr(x+3), Ptr(x+4), Ptr(x+5));
    test(std::move(c1), 1, Ptr(x+3), Ptr(x+4), Ptr(x+5), c2);
    c1.clear();
    for (int i = 0; i < 3; ++i)
        c1.push_back(Ptr(x+i));
    c2.clear();
    for (int i = 0; i < 3; ++i)
        c2.push_back(Ptr(x+i));
    insert3at(c2, c2.begin()+2, Ptr(x+3), Ptr(x+4), Ptr(x+5));
    test(std::move(c1), 2, Ptr(x+3), Ptr(x+4), Ptr(x+5), c2);
    c1.clear();
    for (int i = 0; i < 3; ++i)
        c1.push_back(Ptr(x+i));
    c2.clear();
    for (int i = 0; i < 3; ++i)
        c2.push_back(Ptr(x+i));
    insert3at(c2, c2.begin()+3, Ptr(x+3), Ptr(x+4), Ptr(x+5));
    test(std::move(c1), 3, Ptr(x+3), Ptr(x+4), Ptr(x+5), c2);
    }
}
