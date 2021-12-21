//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// template <class C> auto begin(C& c) -> decltype(c.begin());

#include <vector>
#include <cassert>

int main()
{
    int ia[] = {1, 2, 3};
    std::vector<int> v(ia, ia + sizeof(ia)/sizeof(ia[0]));
    std::vector<int>::iterator i = begin(v);
    assert(*i == 1);
    *i = 2;
    assert(*i == 2);
}
