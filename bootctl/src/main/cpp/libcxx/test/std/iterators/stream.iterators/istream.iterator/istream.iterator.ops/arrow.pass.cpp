//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// class istream_iterator

// const T* operator->() const;

#include <iterator>
#include <sstream>
#include <cassert>

struct A
{
    double d_;
    int i_;
};

void operator&(A const&) {}

std::istream& operator>>(std::istream& is, A& a)
{
    return is >> a.d_ >> a.i_;
}

int main()
{
    std::istringstream inf("1.5  23 ");
    std::istream_iterator<A> i(inf);
    assert(i->d_ == 1.5);
    assert(i->i_ == 23);
}
