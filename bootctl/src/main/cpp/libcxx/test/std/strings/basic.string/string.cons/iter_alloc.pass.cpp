//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<class InputIterator>
//   basic_string(InputIterator begin, InputIterator end,
//   const Allocator& a = Allocator());


#include <string>
#include <iterator>
#include <cassert>
#include <cstddef>

#include "test_macros.h"
#include "test_allocator.h"
#include "../input_iterator.h"
#include "min_allocator.h"

template <class It>
void
test(It first, It last)
{
    typedef typename std::iterator_traits<It>::value_type charT;
    typedef std::basic_string<charT, std::char_traits<charT>, test_allocator<charT> > S;
    typedef typename S::allocator_type A;
    S s2(first, last);
    LIBCPP_ASSERT(s2.__invariants());
    assert(s2.size() == static_cast<std::size_t>(std::distance(first, last)));
    unsigned i = 0;
    for (It it = first; it != last; ++it, ++i)
        assert(s2[i] == *it);
    assert(s2.get_allocator() == A());
    assert(s2.capacity() >= s2.size());
}

template <class It, class A>
void
test(It first, It last, const A& a)
{
    typedef typename std::iterator_traits<It>::value_type charT;
    typedef std::basic_string<charT, std::char_traits<charT>, A> S;
    S s2(first, last, a);
    LIBCPP_ASSERT(s2.__invariants());
    assert(s2.size() == static_cast<std::size_t>(std::distance(first, last)));
    unsigned i = 0;
    for (It it = first; it != last; ++it, ++i)
        assert(s2[i] == *it);
    assert(s2.get_allocator() == a);
    assert(s2.capacity() >= s2.size());
}

int main()
{
    {
    typedef test_allocator<char> A;
    const char* s = "12345678901234567890123456789012345678901234567890";

    test(s, s);
    test(s, s, A(2));

    test(s, s+1);
    test(s, s+1, A(2));

    test(s, s+10);
    test(s, s+10, A(2));

    test(s, s+50);
    test(s, s+50, A(2));

    test(input_iterator<const char*>(s), input_iterator<const char*>(s));
    test(input_iterator<const char*>(s), input_iterator<const char*>(s), A(2));

    test(input_iterator<const char*>(s), input_iterator<const char*>(s+1));
    test(input_iterator<const char*>(s), input_iterator<const char*>(s+1), A(2));

    test(input_iterator<const char*>(s), input_iterator<const char*>(s+10));
    test(input_iterator<const char*>(s), input_iterator<const char*>(s+10), A(2));

    test(input_iterator<const char*>(s), input_iterator<const char*>(s+50));
    test(input_iterator<const char*>(s), input_iterator<const char*>(s+50), A(2));
    }
#if TEST_STD_VER >= 11
    {
    typedef min_allocator<char> A;
    const char* s = "12345678901234567890123456789012345678901234567890";

    test(s, s);
    test(s, s, A());

    test(s, s+1);
    test(s, s+1, A());

    test(s, s+10);
    test(s, s+10, A());

    test(s, s+50);
    test(s, s+50, A());

    test(input_iterator<const char*>(s), input_iterator<const char*>(s));
    test(input_iterator<const char*>(s), input_iterator<const char*>(s), A());

    test(input_iterator<const char*>(s), input_iterator<const char*>(s+1));
    test(input_iterator<const char*>(s), input_iterator<const char*>(s+1), A());

    test(input_iterator<const char*>(s), input_iterator<const char*>(s+10));
    test(input_iterator<const char*>(s), input_iterator<const char*>(s+10), A());

    test(input_iterator<const char*>(s), input_iterator<const char*>(s+50));
    test(input_iterator<const char*>(s), input_iterator<const char*>(s+50), A());
    }
#endif
}
