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

// constexpr reverse_iterator& operator++();
//
// constexpr in C++17

#include <iterator>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"

template <class It>
void
test(It i, It x)
{
    std::reverse_iterator<It> r(i);
    std::reverse_iterator<It>& rr = ++r;
    assert(r.base() == x);
    assert(&rr == &r);
}

int main()
{
    const char* s = "123";
    test(bidirectional_iterator<const char*>(s+1), bidirectional_iterator<const char*>(s));
    test(random_access_iterator<const char*>(s+1), random_access_iterator<const char*>(s));
    test(s+1, s);

#if TEST_STD_VER > 14
    {
        constexpr const char *p = "123456789";
        typedef std::reverse_iterator<const char *> RI;
        constexpr RI it1 = std::make_reverse_iterator(p);
        constexpr RI it2 = std::make_reverse_iterator(p+1);
        static_assert(it1 != it2, "");
        constexpr RI it3 = ++ std::make_reverse_iterator(p+1);
        static_assert(it1 == it3, "");
        static_assert(it2 != it3, "");
        static_assert(*(++std::make_reverse_iterator(p+2)) == '1', "");
    }
#endif
}
