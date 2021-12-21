//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>

// Call erase(const_iterator position) with iterator from another container

#if _LIBCPP_DEBUG >= 1

#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))

#include <unordered_set>
#include <cassert>
#include <cstdlib>
#include <exception>

int main()
{
    {
    int a1[] = {1, 2, 3};
    std::unordered_multiset<int> l1(a1, a1+3);
    std::unordered_multiset<int> l2(a1, a1+3);
    std::unordered_multiset<int>::const_iterator i = l2.begin();
    l1.erase(i);
    assert(false);
    }
}

#else

int main()
{
}

#endif
