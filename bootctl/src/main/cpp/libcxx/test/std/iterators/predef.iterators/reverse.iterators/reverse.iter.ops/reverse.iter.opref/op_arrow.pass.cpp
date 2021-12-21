//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// reverse_iterator

// constexpr pointer operator->() const;
//
// constexpr in C++17

// Be sure to respect LWG 198:
//    http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#198
// LWG 198 was superseded by LWG 2360
//    http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2360


#include <iterator>
#include <list>
#include <cassert>

#include "test_macros.h"

class A
{
    int data_;
public:
    A() : data_(1) {}
    ~A() {data_ = -1;}

    int get() const {return data_;}

    friend bool operator==(const A& x, const A& y)
        {return x.data_ == y.data_;}
};

template <class It>
void
test(It i, typename std::iterator_traits<It>::value_type x)
{
    std::reverse_iterator<It> r(i);
    assert(r->get() == x.get());
}

class B
{
    int data_;
public:
    B(int d=1) : data_(d) {}
    ~B() {data_ = -1;}

    int get() const {return data_;}

    friend bool operator==(const B& x, const B& y)
        {return x.data_ == y.data_;}
    const B *operator&() const { return nullptr; }
    B       *operator&()       { return nullptr; }
};

class C
{
    int data_;
public:
    TEST_CONSTEXPR C() : data_(1) {}

    TEST_CONSTEXPR int get() const {return data_;}

    friend TEST_CONSTEXPR bool operator==(const C& x, const C& y)
        {return x.data_ == y.data_;}
};

TEST_CONSTEXPR  C gC;

int main()
{
    A a;
    test(&a+1, A());

    {
    std::list<B> l;
    l.push_back(B(0));
    l.push_back(B(1));
    l.push_back(B(2));

    {
    std::list<B>::const_iterator i = l.begin();
    assert ( i->get() == 0 );  ++i;
    assert ( i->get() == 1 );  ++i;
    assert ( i->get() == 2 );  ++i;
    assert ( i == l.end ());
    }

    {
    std::list<B>::const_reverse_iterator ri = l.rbegin();
    assert ( ri->get() == 2 );  ++ri;
    assert ( ri->get() == 1 );  ++ri;
    assert ( ri->get() == 0 );  ++ri;
    assert ( ri == l.rend ());
    }
    }

#if TEST_STD_VER > 14
    {
        typedef std::reverse_iterator<const C *> RI;
        constexpr RI it1 = std::make_reverse_iterator(&gC+1);

        static_assert(it1->get() == gC.get(), "");
    }
#endif
    {
        ((void)gC);
    }
}
