//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <sstream>

// template <class charT, class traits = char_traits<charT>, class Allocator = allocator<charT> >
// class basic_istringstream

// basic_istringstream& operator=(basic_istringstream&& rhs);

#include <sstream>
#include <cassert>

int main()
{
    {
        std::istringstream ss0(" 123 456");
        std::istringstream ss;
        ss = std::move(ss0);
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == " 123 456");
        int i = 0;
        ss >> i;
        assert(i == 123);
        ss >> i;
        assert(i == 456);
    }
    {
        std::istringstream s1("Aaaaa Bbbbb Cccccccccc Dddddddddddddddddd");
        std::string s;
        s1 >> s;

        std::istringstream s2 = std::move(s1);
        s2 >> s;
        assert(s == "Bbbbb");

        std::istringstream s3;
        s3 = std::move(s2);
        s3 >> s;
        assert(s == "Cccccccccc");

        s1 = std::move(s3);
        s1 >> s;
        assert(s == "Dddddddddddddddddd");
    }
    {
        std::wistringstream ss0(L" 123 456");
        std::wistringstream ss;
        ss = std::move(ss0);
        assert(ss.rdbuf() != 0);
        assert(ss.good());
        assert(ss.str() == L" 123 456");
        int i = 0;
        ss >> i;
        assert(i == 123);
        ss >> i;
        assert(i == 456);
    }
    {
        std::wistringstream s1(L"Aaaaa Bbbbb Cccccccccc Dddddddddddddddddd");
        std::wstring s;
        s1 >> s;

        std::wistringstream s2 = std::move(s1);
        s2 >> s;
        assert(s == L"Bbbbb");

        std::wistringstream s3;
        s3 = std::move(s2);
        s3 >> s;
        assert(s == L"Cccccccccc");

        s1 = std::move(s3);
        s1 >> s;
        assert(s == L"Dddddddddddddddddd");
    }
}
