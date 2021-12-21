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
// class basic_ofstream

// basic_ofstream();

#include <fstream>
#include <type_traits>

int main()
{
    {
        std::ofstream fs;
    }
    {
        std::wofstream fs;
    }
}
