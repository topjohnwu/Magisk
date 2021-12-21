//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_map>

// Call erase(const_iterator position) with iterator from another container

#if _LIBCPP_DEBUG >= 1

#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))

#include <unordered_map>
#include <cassert>
#include <cstdlib>
#include <exception>

int main()
{
    {
    typedef std::pair<int, int> P;
    P a1[] = {P(1, 1), P(2, 2), P(3, 3)};
    std::unordered_multimap<int, int> l1(a1, a1+3);
    std::unordered_multimap<int, int> l2(a1, a1+3);
    std::unordered_multimap<int, int>::const_iterator i = l2.begin();
    l1.erase(i);
    assert(false);
    }
}

#else

int main()
{
}

#endif
