//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// class ostreambuf_iterator

// ostreambuf_iterator<charT,traits>&
//   operator=(charT c);

#include <iterator>
#include <sstream>
#include <cassert>

int main()
{
    {
        std::ostringstream outf;
        std::ostreambuf_iterator<char> i(outf);
        i = 'a';
        assert(outf.str() == "a");
        i = 'b';
        assert(outf.str() == "ab");
    }
    {
        std::wostringstream outf;
        std::ostreambuf_iterator<wchar_t> i(outf);
        i = L'a';
        assert(outf.str() == L"a");
        i = L'b';
        assert(outf.str() == L"ab");
    }
}
