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

// explicit basic_fstream(const string& s, ios_base::openmode mode = ios_base::in|ios_base::out);

#include <fstream>
#include <cassert>
#include "platform_support.h"

int main()
{
    std::string temp = get_temp_file_name();
    {
        std::fstream fs(temp,
                        std::ios_base::in | std::ios_base::out
                                          | std::ios_base::trunc);
        double x = 0;
        fs << 3.25;
        fs.seekg(0);
        fs >> x;
        assert(x == 3.25);
    }
    std::remove(temp.c_str());
    {
        std::wfstream fs(temp,
                         std::ios_base::in | std::ios_base::out
                                           | std::ios_base::trunc);
        double x = 0;
        fs << 3.25;
        fs.seekg(0);
        fs >> x;
        assert(x == 3.25);
    }
    std::remove(temp.c_str());
}
