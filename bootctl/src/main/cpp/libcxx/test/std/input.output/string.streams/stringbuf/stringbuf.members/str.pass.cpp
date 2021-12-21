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

// void str(const basic_string<charT,traits,Allocator>& s);

#include <sstream>
#include <cassert>

int main()
{
    {
        std::stringbuf buf("testing");
        assert(buf.str() == "testing");
        buf.str("another test");
        assert(buf.str() == "another test");
    }
    {
        std::wstringbuf buf(L"testing");
        assert(buf.str() == L"testing");
        buf.str(L"another test");
        assert(buf.str() == L"another test");
    }
}
