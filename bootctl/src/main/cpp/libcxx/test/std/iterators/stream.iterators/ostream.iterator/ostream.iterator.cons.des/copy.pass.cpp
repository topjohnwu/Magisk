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

// ostream_iterator(const ostream_iterator& x);

#include <iterator>
#include <sstream>
#include <cassert>

int main()
{
    std::ostringstream outf;
    std::ostream_iterator<int> i(outf);
    std::ostream_iterator<int> j = i;
    assert(outf.good());
    ((void)j);
}
