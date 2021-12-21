//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// allocator_type get_allocator() const;

#include <string>
#include <cassert>

#include "test_allocator.h"
#include "min_allocator.h"

template <class S>
void
test(const S& s, const typename S::allocator_type& a)
{
    assert(s.get_allocator() == a);
}

int main()
{
    {
    typedef test_allocator<char> A;
    typedef std::basic_string<char, std::char_traits<char>, A> S;
    test(S(""), A());
    test(S("abcde", A(1)), A(1));
    test(S("abcdefghij", A(2)), A(2));
    test(S("abcdefghijklmnopqrst", A(3)), A(3));
    }
#if TEST_STD_VER >= 11
    {
    typedef min_allocator<char> A;
    typedef std::basic_string<char, std::char_traits<char>, A> S;
    test(S(""), A());
    test(S("abcde", A()), A());
    test(S("abcdefghij", A()), A());
    test(S("abcdefghijklmnopqrst", A()), A());
    }
#endif
}
