//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <regex>

// template <class BidirectionalIterator> class sub_match;

// template <class charT, class ST, class BiIter>
//     basic_ostream<charT, ST>&
//     operator<<(basic_ostream<charT, ST>& os, const sub_match<BiIter>& m);

#include <regex>
#include <sstream>
#include <cassert>
#include "test_macros.h"

template <class CharT>
void
test(const std::basic_string<CharT>& s)
{
    typedef std::basic_string<CharT> string;
    typedef std::sub_match<typename string::const_iterator> SM;
    typedef std::basic_ostringstream<CharT> ostringstream;
    SM sm;
    sm.first = s.begin();
    sm.second = s.end();
    sm.matched = true;
    ostringstream os;
    os << sm;
    assert(os.str() == s);
}

int main()
{
    test(std::string("123"));
    test(std::wstring(L"123"));
}
