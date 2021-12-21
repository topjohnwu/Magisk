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
// class basic_fstream

// basic_filebuf<charT,traits>* rdbuf() const;

#include <fstream>
#include <cassert>

int main()
{
    {
        std::fstream fs;
        assert(fs.rdbuf());
    }
    {
        std::wfstream fs;
        assert(fs.rdbuf());
    }
}
