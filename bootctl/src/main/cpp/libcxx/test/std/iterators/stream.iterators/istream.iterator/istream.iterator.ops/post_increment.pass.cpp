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

// istream_iterator operator++(int);

#include <iterator>
#include <sstream>
#include <cassert>

int main()
{
    std::istringstream inf(" 1 23");
    std::istream_iterator<int> i(inf);
    std::istream_iterator<int> icopy = i++;
    assert(icopy == i);
    int j = 0;
    j = *i;
    assert(j == 23);
    j = 0;
    j = *icopy;
    assert(j == 1);
}
