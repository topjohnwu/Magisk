//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// move_iterator

// explicit move_iterator(Iter );

// test explicit

#include <iterator>

template <class It>
void
test(It i)
{
    std::move_iterator<It> r = i;
}

int main()
{
    char s[] = "123";
    test(s);
}
