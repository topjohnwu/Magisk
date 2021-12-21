//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <string>

// template<class charT, class traits, class Allocator>
//   bool operator<(const basic_string<charT,traits,Allocator>& lhs,
//                   basic_string_view<charT,traits> rhs);
//   bool operator<(basic_string_view<charT,traits> lhs,
//            const basic_string<charT,traits,Allocator>&  rhs);

#include <string_view>
#include <cassert>

template <class S>
void
test(const S& lhs, const typename S::value_type* rhs, bool x, bool y)
{
    assert((lhs < rhs) == x);
    assert((rhs < lhs) == y);
}

int main()
{
    {
    typedef std::string_view S;
    test(S(""), "", false, false);
    test(S(""), "abcde", true, false);
    test(S(""), "abcdefghij", true, false);
    test(S(""), "abcdefghijklmnopqrst", true, false);
    test(S("abcde"), "", false, true);
    test(S("abcde"), "abcde", false, false);
    test(S("abcde"), "abcdefghij", true, false);
    test(S("abcde"), "abcdefghijklmnopqrst", true, false);
    test(S("abcdefghij"), "", false, true);
    test(S("abcdefghij"), "abcde", false, true);
    test(S("abcdefghij"), "abcdefghij", false, false);
    test(S("abcdefghij"), "abcdefghijklmnopqrst", true, false);
    test(S("abcdefghijklmnopqrst"), "", false, true);
    test(S("abcdefghijklmnopqrst"), "abcde", false, true);
    test(S("abcdefghijklmnopqrst"), "abcdefghij", false, true);
    test(S("abcdefghijklmnopqrst"), "abcdefghijklmnopqrst", false, false);
    }
}
