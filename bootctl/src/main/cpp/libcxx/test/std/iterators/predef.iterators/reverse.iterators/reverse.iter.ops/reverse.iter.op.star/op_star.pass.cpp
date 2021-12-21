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

// constexpr reference operator*() const;
//
// constexpr in C++17

// Be sure to respect LWG 198:
//    http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#198
// LWG 198 was superseded by LWG 2360
//    http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2360

#include <iterator>
#include <cassert>

#include "test_macros.h"

class A
{
    int data_;
public:
    A() : data_(1) {}
    ~A() {data_ = -1;}

    friend bool operator==(const A& x, const A& y)
        {return x.data_ == y.data_;}
};

template <class It>
void
test(It i, typename std::iterator_traits<It>::value_type x)
{
    std::reverse_iterator<It> r(i);
    assert(*r == x);
}

int main()
{
    A a;
    test(&a+1, A());

#if TEST_STD_VER > 14
    {
        constexpr const char *p = "123456789";
        typedef std::reverse_iterator<const char *> RI;
        constexpr RI it1 = std::make_reverse_iterator(p+1);
        constexpr RI it2 = std::make_reverse_iterator(p+2);
        static_assert(*it1 == p[0], "");
        static_assert(*it2 == p[1], "");
    }
#endif
}
