//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <> class ctype<char>;

// const char* toupper(char* low, const char* high) const;

#include <locale>
#include <string>
#include <cassert>

int main()
{
    std::locale l = std::locale::classic();
    {
        typedef std::ctype<char> F;
        const F& f = std::use_facet<F>(l);
        std::string in(" A\x07.a1");

        assert(f.toupper(&in[0], in.data() + in.size()) == in.data() + in.size());
        assert(in[0] == ' ');
        assert(in[1] == 'A');
        assert(in[2] == '\x07');
        assert(in[3] == '.');
        assert(in[4] == 'A');
        assert(in[5] == '1');
    }
}
