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
// class basic_stringstream

// explicit basic_stringstream(ios_base::openmode which = ios_base::out|ios_base::in);

#include <sstream>
#include <cassert>

int main()
{
    {
        std::stringstream ss;
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == "");
    }
    {
        std::stringstream ss(std::ios_base::in);
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == "");
    }
    {
        std::wstringstream ss;
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == L"");
    }
    {
        std::wstringstream ss(std::ios_base::in);
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == L"");
    }
}
