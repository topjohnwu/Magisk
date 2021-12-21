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

// explicit basic_stringbuf(ios_base::openmode which = ios_base::in | ios_base::out);

#include <sstream>
#include <cassert>

template<typename CharT>
struct testbuf
    : std::basic_stringbuf<CharT>
{
    void check()
    {
        assert(this->eback() == NULL);
        assert(this->gptr() == NULL);
        assert(this->egptr() == NULL);
        assert(this->pbase() == NULL);
        assert(this->pptr() == NULL);
        assert(this->epptr() == NULL);
    }
};

int main()
{
    {
        std::stringbuf buf;
        assert(buf.str() == "");
    }
    {
        std::wstringbuf buf;
        assert(buf.str() == L"");
    }
    {
        testbuf<char> buf;
        buf.check();
    }
    {
        testbuf<wchar_t> buf;
        buf.check();
    }
}
