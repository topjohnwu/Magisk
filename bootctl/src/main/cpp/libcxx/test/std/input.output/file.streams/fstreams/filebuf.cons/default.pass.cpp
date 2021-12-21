//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <fstream>

// template <class charT, class traits = char_traits<charT> >
// class basic_filebuf

// basic_filebuf();

#include <fstream>
#include <cassert>

int main()
{
    {
        std::filebuf f;
        assert(!f.is_open());
    }
    {
        std::wfilebuf f;
        assert(!f.is_open());
    }
}
