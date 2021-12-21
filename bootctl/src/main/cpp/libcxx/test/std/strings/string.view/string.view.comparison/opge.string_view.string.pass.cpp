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
//   bool operator>=(const basic_string<charT,traits,Allocator>& lhs,
//                   basic_string_view<charT,traits> rhs);
//   bool operator>=(basic_string_view<charT,traits> lhs,
//            const basic_string<charT,traits,Allocator>&  rhs);

#include <string_view>
#include <cassert>

template <class S>
void
test(const S& lhs, const typename S::value_type* rhs, bool x, bool y)
{
    assert((lhs >= rhs) == x);
    assert((rhs >= lhs) == y);
}

int main()
{
    {
    typedef std::string_view S;
    test(S(""), "", true, true);
    test(S(""), "abcde", false, true);
    test(S(""), "abcdefghij", false, true);
    test(S(""), "abcdefghijklmnopqrst", false, true);
    test(S("abcde"), "", true, false);
    test(S("abcde"), "abcde", true, true);
    test(S("abcde"), "abcdefghij", false, true);
    test(S("abcde"), "abcdefghijklmnopqrst", false, true);
    test(S("abcdefghij"), "", true, false);
    test(S("abcdefghij"), "abcde", true, false);
    test(S("abcdefghij"), "abcdefghij", true, true);
    test(S("abcdefghij"), "abcdefghijklmnopqrst", false, true);
    test(S("abcdefghijklmnopqrst"), "", true, false);
    test(S("abcdefghijklmnopqrst"), "abcde", true, false);
    test(S("abcdefghijklmnopqrst"), "abcdefghij", true, false);
    test(S("abcdefghijklmnopqrst"), "abcdefghijklmnopqrst", true, true);
    }
}
