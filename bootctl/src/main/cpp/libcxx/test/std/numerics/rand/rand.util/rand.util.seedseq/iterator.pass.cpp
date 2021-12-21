//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// class seed_seq;

// template<class InputIterator>
//     seed_seq(InputIterator begin, InputIterator end);

#include <random>
#include <cassert>

int main()
{
    unsigned a[5] = {5, 4, 3, 2, 1};
    std::seed_seq s(a, a+5);
    assert(s.size() == 5);
    unsigned b[5] = {0};
    s.param(b);
    assert(b[0] == 5);
    assert(b[1] == 4);
    assert(b[2] == 3);
    assert(b[3] == 2);
    assert(b[4] == 1);
}
