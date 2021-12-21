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
// class basic_ifstream

// basic_ifstream(basic_ifstream&& rhs);

#include <fstream>
#include <cassert>

int main()
{
    {
        std::ifstream fso("test.dat");
        std::ifstream fs = move(fso);
        double x = 0;
        fs >> x;
        assert(x == 3.25);
    }
    {
        std::wifstream fso("test.dat");
        std::wifstream fs = move(fso);
        double x = 0;
        fs >> x;
        assert(x == 3.25);
    }
}
