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

// ostream_iterator& operator*() const;

#include <iterator>
#include <sstream>
#include <cassert>

int main()
{
    std::ostringstream os;
    std::ostream_iterator<int> i(os);
    std::ostream_iterator<int>& iref = *i;
    assert(&iref == &i);
}
