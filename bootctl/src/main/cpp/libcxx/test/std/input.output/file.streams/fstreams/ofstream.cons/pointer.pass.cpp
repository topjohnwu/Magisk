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

// explicit basic_ofstream(const char* s, ios_base::openmode mode = ios_base::out);

#include <fstream>
#include <cassert>
#include "platform_support.h"

int main()
{
    std::string temp = get_temp_file_name();
    {
        std::ofstream fs(temp.c_str());
        fs << 3.25;
    }
    {
        std::ifstream fs(temp.c_str());
        double x = 0;
        fs >> x;
        assert(x == 3.25);
    }
    {
        std::ifstream fs(temp.c_str(), std::ios_base::out);
        double x = 0;
        fs >> x;
        assert(x == 3.25);
    }
    std::remove(temp.c_str());
    {
        std::wofstream fs(temp.c_str());
        fs << 3.25;
    }
    {
        std::wifstream fs(temp.c_str());
        double x = 0;
        fs >> x;
        assert(x == 3.25);
    }
    {
        std::wifstream fs(temp.c_str(), std::ios_base::out);
        double x = 0;
        fs >> x;
        assert(x == 3.25);
    }
    std::remove(temp.c_str());
}
