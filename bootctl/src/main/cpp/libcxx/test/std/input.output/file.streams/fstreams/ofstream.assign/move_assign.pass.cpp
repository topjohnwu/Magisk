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
// class basic_ofstream

// basic_ofstream& operator=(basic_ofstream&& rhs);

#include <fstream>
#include <cassert>
#include "platform_support.h"

int main()
{
    std::string temp = get_temp_file_name();
    {
        std::ofstream fso(temp.c_str());
        std::ofstream fs;
        fs = move(fso);
        fs << 3.25;
    }
    {
        std::ifstream fs(temp.c_str());
        double x = 0;
        fs >> x;
        assert(x == 3.25);
    }
    std::remove(temp.c_str());
    {
        std::wofstream fso(temp.c_str());
        std::wofstream fs;
        fs = move(fso);
        fs << 3.25;
    }
    {
        std::wifstream fs(temp.c_str());
        double x = 0;
        fs >> x;
        assert(x == 3.25);
    }
    std::remove(temp.c_str());
}
