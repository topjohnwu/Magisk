//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// UNSUPPORTED: c++98, c++03
// <regex>

// class match_results<BidirectionalIterator, Allocator>

// match_results(match_results&& m) noexcept;
//
//  Additionally, the stored Allocator value is move constructed from m.get_allocator().

#include <regex>
#include <cassert>
#include "test_macros.h"
#include "test_allocator.h"

template <class CharT, class Allocator>
void
test(const Allocator& a)
{
    typedef std::match_results<const CharT*, Allocator> SM;
    ASSERT_NOEXCEPT(SM(std::declval<SM&&>()));

    SM m0(a);
    assert(m0.get_allocator() == a);

    SM m1(std::move(m0));
    assert(m1.size() == 0);
    assert(m1.str() == std::basic_string<CharT>());
    assert(m1.get_allocator() == a);
}

int main()
{
    test<char>   (std::allocator<std::sub_match<const char *> >());
    test<wchar_t>(std::allocator<std::sub_match<const wchar_t *> >());

    test<char>   (test_allocator<std::sub_match<const char*> >(3));
    assert(test_alloc_base::moved == 1);
    test<wchar_t>(test_allocator<std::sub_match<const wchar_t*> >(3));
    assert(test_alloc_base::moved == 2);
}
