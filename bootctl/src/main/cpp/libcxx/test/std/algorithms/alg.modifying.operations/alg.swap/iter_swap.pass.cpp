//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <algorithm>

// template<Iterator Iter1, Iterator Iter2>
//   requires HasSwap<Iter1::reference, Iter2::reference>
//   void
//   iter_swap(Iter1 a, Iter2 b);

#include <algorithm>
#include <cassert>

int main()
{
    int i = 1;
    int j = 2;
    std::iter_swap(&i, &j);
    assert(i == 2);
    assert(j == 1);
}
