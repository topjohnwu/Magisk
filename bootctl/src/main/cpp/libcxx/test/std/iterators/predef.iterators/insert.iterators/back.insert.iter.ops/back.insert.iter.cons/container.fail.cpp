//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// back_insert_iterator

// explicit back_insert_iterator(Cont& x);

// test for explicit

#include <iterator>
#include <vector>

int main()
{
    std::back_insert_iterator<std::vector<int> > i = std::vector<int>();
}
