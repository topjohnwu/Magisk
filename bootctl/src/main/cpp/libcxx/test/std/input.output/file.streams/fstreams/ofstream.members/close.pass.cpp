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

// void close();

#include <fstream>
#include <cassert>
#include "platform_support.h"

int main()
{
    std::string temp = get_temp_file_name();
    {
        std::ofstream fs;
        assert(!fs.is_open());
        fs.open(temp.c_str());
        assert(fs.is_open());
        fs.close();
        assert(!fs.is_open());
    }
    std::remove(temp.c_str());
    {
        std::wofstream fs;
        assert(!fs.is_open());
        fs.open(temp.c_str());
        assert(fs.is_open());
        fs.close();
        assert(!fs.is_open());
    }
    std::remove(temp.c_str());
}
