//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// reverse_iterator

// explicit reverse_iterator(Iter x);

// test explicit

#include <iterator>

template <class It>
void
test(It i)
{
    std::reverse_iterator<It> r = i;
}

int main()
{
    const char s[] = "123";
    test(s);
}
