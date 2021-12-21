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
// class basic_ifstream

// template <class charT, class traits>
//   void swap(basic_ifstream<charT, traits>& x, basic_ifstream<charT, traits>& y);

#include <fstream>
#include <cassert>

int main()
{
    {
        std::ifstream fs1("test.dat");
        std::ifstream fs2("test2.dat");
        swap(fs1, fs2);
        double x = 0;
        fs1 >> x;
        assert(x == 4.5);
        fs2 >> x;
        assert(x == 3.25);
    }
    {
        std::wifstream fs1("test.dat");
        std::wifstream fs2("test2.dat");
        swap(fs1, fs2);
        double x = 0;
        fs1 >> x;
        assert(x == 4.5);
        fs2 >> x;
        assert(x == 3.25);
    }
}
