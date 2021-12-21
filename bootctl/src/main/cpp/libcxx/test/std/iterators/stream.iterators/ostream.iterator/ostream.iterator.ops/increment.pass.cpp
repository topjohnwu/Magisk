//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// class ostream_iterator

// ostream_iterator& operator++();
// ostream_iterator& operator++(int);

#include <iterator>
#include <sstream>
#include <cassert>

int main()
{
    std::ostringstream os;
    std::ostream_iterator<int> i(os);
    std::ostream_iterator<int>& iref1 = ++i;
    assert(&iref1 == &i);
    std::ostream_iterator<int>& iref2 = i++;
    assert(&iref2 == &i);
}
