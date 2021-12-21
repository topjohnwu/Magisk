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

// explicit move_iterator(Iter i);
//
//  constexpr in C++17

#include <iterator>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

template <class It>
void
test(It i)
{
    std::move_iterator<It> r(i);
    assert(r.base() == i);
}

int main()
{
    char s[] = "123";
    test(input_iterator<char*>(s));
    test(forward_iterator<char*>(s));
    test(bidirectional_iterator<char*>(s));
    test(random_access_iterator<char*>(s));
    test(s);

#if TEST_STD_VER > 14
    {
    constexpr const char *p = "123456789";
    constexpr std::move_iterator<const char *> it(p);
    static_assert(it.base() == p);
    }
#endif
}
