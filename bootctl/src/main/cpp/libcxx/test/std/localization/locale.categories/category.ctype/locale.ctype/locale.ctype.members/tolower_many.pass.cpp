//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class charT> class ctype;

// const charT* tolower(charT* low, const charT* high) const;

#include <locale>
#include <string>
#include <cassert>

int main()
{
    std::locale l = std::locale::classic();
    {
        typedef std::ctype<wchar_t> F;
        const F& f = std::use_facet<F>(l);
        std::wstring in(L" A\x07.a1");

        assert(f.tolower(&in[0], in.data() + in.size()) == in.data() + in.size());
        assert(in[0] == L' ');
        assert(in[1] == L'a');
        assert(in[2] == L'\x07');
        assert(in[3] == L'.');
        assert(in[4] == L'a');
        assert(in[5] == L'1');
    }
}
