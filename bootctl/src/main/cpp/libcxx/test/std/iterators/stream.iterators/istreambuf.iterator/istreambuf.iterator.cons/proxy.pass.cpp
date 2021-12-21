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

// istreambuf_iterator(const proxy& p) throw();

#include <iterator>
#include <sstream>
#include <cassert>

int main()
{
    {
        std::istringstream inf("abc");
        std::istreambuf_iterator<char> j(inf);
        std::istreambuf_iterator<char> i = j++;
        assert(i != std::istreambuf_iterator<char>());
        assert(*i == 'b');
    }
    {
        std::wistringstream inf(L"abc");
        std::istreambuf_iterator<wchar_t> j(inf);
        std::istreambuf_iterator<wchar_t> i = j++;
        assert(i != std::istreambuf_iterator<wchar_t>());
        assert(*i == L'b');
    }
}
