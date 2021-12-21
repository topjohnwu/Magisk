//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: libcpp-no-exceptions
// <string>

// size_type max_size() const;

// NOTE: asan and msan will fail for one of two reasons
// 1. If allocator_may_return_null=0 then they will fail because the allocation
//    returns null.
// 2. If allocator_may_return_null=1 then they will fail because the allocation
//    is too large to succeed.
// UNSUPPORTED: sanitizer-new-delete

#include <string>
#include <cassert>

#include "min_allocator.h"

template <class S>
void
test1(const S& s)
{
    S s2(s);
    const size_t sz = s2.max_size() - 1;
    try { s2.resize(sz, 'x'); }
    catch ( const std::bad_alloc & ) { return ; }
    assert ( s2.size() ==  sz );
}

template <class S>
void
test2(const S& s)
{
    S s2(s);
    const size_t sz = s2.max_size();
    try { s2.resize(sz, 'x'); }
    catch ( const std::bad_alloc & ) { return ; }
    assert ( s.size() ==  sz );
}

template <class S>
void
test(const S& s)
{
    assert(s.max_size() >= s.size());
    test1(s);
    test2(s);
}

int main()
{
    {
    typedef std::string S;
    test(S());
    test(S("123"));
    test(S("12345678901234567890123456789012345678901234567890"));
    }
#if TEST_STD_VER >= 11
    {
    typedef std::basic_string<char, std::char_traits<char>, min_allocator<char>> S;
    test(S());
    test(S("123"));
    test(S("12345678901234567890123456789012345678901234567890"));
    }
#endif
}
