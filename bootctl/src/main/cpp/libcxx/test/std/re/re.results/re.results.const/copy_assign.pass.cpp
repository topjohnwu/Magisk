//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// class match_results<BidirectionalIterator, Allocator>

// match_results& operator=(const match_results& m);

#include <regex>
#include <cassert>
#include "test_macros.h"
#include "test_allocator.h"

template <class CharT, class Allocator>
void
test(const Allocator& a)
{
    typedef std::match_results<const CharT*, Allocator> SM;
    SM m0(a);
    SM m1;

    m1 = m0;
    assert(m1.size()          == m0.size());
    assert(m1.str()           == m0.str());
    if (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value)
        assert(m1.get_allocator() == m0.get_allocator());
    else
        assert(m1.get_allocator() == Allocator());
}

int main()
{
    test<char>   (std::allocator<std::sub_match<const char *> >());
    test<wchar_t>(std::allocator<std::sub_match<const wchar_t *> >());

//  test_allocator has POCCA -> false
    test<char>   (test_allocator<std::sub_match<const char*> >(3));
    test<wchar_t>(test_allocator<std::sub_match<const wchar_t*> >(3));

//  other_allocator has POCCA -> true
    test<char>   (other_allocator<std::sub_match<const char*> >(3));
    test<wchar_t>(other_allocator<std::sub_match<const wchar_t*> >(3));
}
