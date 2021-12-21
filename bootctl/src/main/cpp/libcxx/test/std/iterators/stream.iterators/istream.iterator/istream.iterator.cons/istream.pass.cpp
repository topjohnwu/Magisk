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

// istream_iterator(istream_type& s);

#include <iterator>
#include <sstream>
#include <cassert>

int main()
{
    std::istringstream inf(" 1 23");
    std::istream_iterator<int> i(inf);
    assert(i != std::istream_iterator<int>());
    assert(inf.peek() == ' ');
    assert(inf.good());
    int j = 0;
    inf >> j;
    assert(j == 23);
}
