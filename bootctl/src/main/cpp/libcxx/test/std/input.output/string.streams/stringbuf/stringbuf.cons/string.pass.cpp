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

// explicit basic_stringbuf(const basic_string<charT,traits,Allocator>& s,
//                          ios_base::openmode which = ios_base::in | ios_base::out);

#include <sstream>
#include <cassert>

int main()
{
    {
        std::stringbuf buf("testing");
        assert(buf.str() == "testing");
    }
    {
        std::stringbuf buf("testing", std::ios_base::in);
        assert(buf.str() == "testing");
    }
    {
        std::stringbuf buf("testing", std::ios_base::out);
        assert(buf.str() == "testing");
    }
    {
        std::wstringbuf buf(L"testing");
        assert(buf.str() == L"testing");
    }
    {
        std::wstringbuf buf(L"testing", std::ios_base::in);
        assert(buf.str() == L"testing");
    }
    {
        std::wstringbuf buf(L"testing", std::ios_base::out);
        assert(buf.str() == L"testing");
    }
}
