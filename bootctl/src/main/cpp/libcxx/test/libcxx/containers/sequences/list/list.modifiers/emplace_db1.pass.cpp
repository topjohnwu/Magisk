//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// Can't test the system lib because this test enables debug mode
// UNSUPPORTED: with_system_cxx_lib

// <list>

// template <class... Args> void emplace(const_iterator p, Args&&... args);

#define _LIBCPP_DEBUG 1
#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))

#include <list>
#include <cstdlib>
#include <cassert>

class A
{
    int i_;
    double d_;

    A(const A&);
    A& operator=(const A&);
public:
    A(int i, double d)
        : i_(i), d_(d) {}

    int geti() const {return i_;}
    double getd() const {return d_;}
};

int main()
{
    std::list<A> c1;
    std::list<A> c2;
    std::list<A>::iterator i = c1.emplace(c2.cbegin(), 2, 3.5);
    assert(false);
}
