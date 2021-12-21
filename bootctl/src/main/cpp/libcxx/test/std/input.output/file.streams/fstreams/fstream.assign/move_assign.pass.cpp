//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <fstream>

// template <class charT, class traits = char_traits<charT> >
// class basic_fstream

// basic_fstream& operator=(basic_fstream&& rhs);

#include <fstream>
#include <cassert>
#include "platform_support.h"

int main()
{
    std::string temp = get_temp_file_name();
    {
        std::fstream fso(temp.c_str(), std::ios_base::in | std::ios_base::out
                                                 | std::ios_base::trunc);
        std::fstream fs;
        fs = move(fso);
        double x = 0;
        fs << 3.25;
        fs.seekg(0);
        fs >> x;
        assert(x == 3.25);
    }
    std::remove(temp.c_str());
    {
        std::wfstream fso(temp.c_str(), std::ios_base::in | std::ios_base::out
                                                  | std::ios_base::trunc);
        std::wfstream fs;
        fs = move(fso);
        double x = 0;
        fs << 3.25;
        fs.seekg(0);
        fs >> x;
        assert(x == 3.25);
    }
    std::remove(temp.c_str());
}
