//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <iterator>

// istreambuf_iterator

// istreambuf_iterator(basic_istream<charT,traits>& s) throw();

#include <iterator>
#include <sstream>
#include <cassert>

int main()
{
    {
        std::istringstream inf;
        std::istreambuf_iterator<char> i(inf);
        assert(i == std::istreambuf_iterator<char>());
    }
    {
        std::istringstream inf("a");
        std::istreambuf_iterator<char> i(inf);
        assert(i != std::istreambuf_iterator<char>());
    }
    {
        std::wistringstream inf;
        std::istreambuf_iterator<wchar_t> i(inf);
        assert(i == std::istreambuf_iterator<wchar_t>());
    }
    {
        std::wistringstream inf(L"a");
        std::istreambuf_iterator<wchar_t> i(inf);
        assert(i != std::istreambuf_iterator<wchar_t>());
    }
}
