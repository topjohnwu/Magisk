//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <sstream>

// template <class charT, class traits = char_traits<charT>, class Allocator = allocator<charT> >
// class basic_stringbuf

// basic_streambuf<charT,traits>* setbuf(charT* s, streamsize n);

#include <sstream>
#include <cassert>

int main()
{
    {
        std::stringbuf sb("0123456789");
        assert(sb.pubsetbuf(0, 0) == &sb);
        assert(sb.str() == "0123456789");
    }
    {
        std::wstringbuf sb(L"0123456789");
        assert(sb.pubsetbuf(0, 0) == &sb);
        assert(sb.str() == L"0123456789");
    }
}
