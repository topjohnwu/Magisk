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

// template <class charT, class traits, class Allocator>
//   void swap(basic_stringbuf<charT, traits, Allocator>& x,
//             basic_stringbuf<charT, traits, Allocator>& y);

#include <sstream>
#include <cassert>

int main()
{
    {
        std::stringbuf buf1("testing");
        std::stringbuf buf;
        swap(buf, buf1);
        assert(buf.str() == "testing");
        assert(buf1.str() == "");
    }
    {
        std::stringbuf buf1("testing", std::ios_base::in);
        std::stringbuf buf;
        swap(buf, buf1);
        assert(buf.str() == "testing");
        assert(buf1.str() == "");
    }
    {
        std::stringbuf buf1("testing", std::ios_base::out);
        std::stringbuf buf;
        swap(buf, buf1);
        assert(buf.str() == "testing");
        assert(buf1.str() == "");
    }
    {
        std::wstringbuf buf1(L"testing");
        std::wstringbuf buf;
        swap(buf, buf1);
        assert(buf.str() == L"testing");
        assert(buf1.str() == L"");
    }
    {
        std::wstringbuf buf1(L"testing", std::ios_base::in);
        std::wstringbuf buf;
        swap(buf, buf1);
        assert(buf.str() == L"testing");
        assert(buf1.str() == L"");
    }
    {
        std::wstringbuf buf1(L"testing", std::ios_base::out);
        std::wstringbuf buf;
        swap(buf, buf1);
        assert(buf.str() == L"testing");
        assert(buf1.str() == L"");
    }
}
