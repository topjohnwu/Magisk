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

// charT operator*() const

#include <iterator>
#include <sstream>
#include <cassert>

int main()
{
    {
        std::istringstream inf("abc");
        std::istreambuf_iterator<char> i(inf);
        assert(*i == 'a');
        ++i;
        assert(*i == 'b');
        ++i;
        assert(*i == 'c');
    }
    {
        std::wistringstream inf(L"abc");
        std::istreambuf_iterator<wchar_t> i(inf);
        assert(*i == L'a');
        ++i;
        assert(*i == L'b');
        ++i;
        assert(*i == L'c');
    }
}
