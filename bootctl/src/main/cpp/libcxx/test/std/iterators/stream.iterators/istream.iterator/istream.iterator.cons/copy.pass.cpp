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

// istream_iterator(const istream_iterator& x);
//  C++17 says:  If is_trivially_copy_constructible_v<T> is true, then
//     this constructor is a trivial copy constructor.

#include <iterator>
#include <sstream>
#include <cassert>

#include "test_macros.h"

int main()
{
    {
        std::istream_iterator<int> io;
        std::istream_iterator<int> i = io;
        assert(i == std::istream_iterator<int>());
    }
    {
        std::istringstream inf(" 1 23");
        std::istream_iterator<int> io(inf);
        std::istream_iterator<int> i = io;
        assert(i != std::istream_iterator<int>());
        int j = 0;
        j = *i;
        assert(j == 1);
    }
}
