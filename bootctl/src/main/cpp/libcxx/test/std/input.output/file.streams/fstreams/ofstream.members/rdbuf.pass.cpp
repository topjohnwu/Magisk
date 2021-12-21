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

// basic_filebuf<charT,traits>* rdbuf() const;

#include <fstream>
#include <cassert>
#include "platform_support.h"

int main()
{
    std::string temp = get_temp_file_name();
    {
        std::ofstream fs(temp.c_str());
        std::filebuf* fb = fs.rdbuf();
        assert(fb->sputc('r') == 'r');
    }
    std::remove(temp.c_str());
    {
        std::wofstream fs(temp.c_str());
        std::wfilebuf* fb = fs.rdbuf();
        assert(fb->sputc(L'r') == L'r');
    }
    std::remove(temp.c_str());
}
