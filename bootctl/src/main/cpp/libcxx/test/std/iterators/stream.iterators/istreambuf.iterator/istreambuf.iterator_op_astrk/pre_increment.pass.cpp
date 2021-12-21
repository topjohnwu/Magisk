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

// istreambuf_iterator<charT,traits>&
//   istreambuf_iterator<charT,traits>::operator++();

#include <iterator>
#include <sstream>
#include <cassert>

int main()
{
    {
        std::istringstream inf("abc");
        std::istreambuf_iterator<char> i(inf);
        assert(*i == 'a');
        assert(*++i == 'b');
        assert(*++i == 'c');
        assert(++i == std::istreambuf_iterator<char>());
    }
    {
        std::wistringstream inf(L"abc");
        std::istreambuf_iterator<wchar_t> i(inf);
        assert(*i == L'a');
        assert(*++i == L'b');
        assert(*++i == L'c');
        assert(++i == std::istreambuf_iterator<wchar_t>());
    }
}
