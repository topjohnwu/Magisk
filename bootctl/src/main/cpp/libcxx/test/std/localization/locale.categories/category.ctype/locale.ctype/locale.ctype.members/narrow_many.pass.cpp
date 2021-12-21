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

// const charT* narrow(const charT* low, const charT*, char dfault, char* to) const;

#include <locale>
#include <string>
#include <vector>
#include <cassert>

int main()
{
    std::locale l = std::locale::classic();
    {
        typedef std::ctype<wchar_t> F;
        const F& f = std::use_facet<F>(l);
        std::wstring in(L" A\x07.a1");
        std::vector<char> v(in.size());

        assert(f.narrow(&in[0], in.data() + in.size(), '*', v.data()) == in.data() + in.size());
        assert(v[0] == ' ');
        assert(v[1] == 'A');
        assert(v[2] == '\x07');
        assert(v[3] == '.');
        assert(v[4] == 'a');
        assert(v[5] == '1');
    }
}
