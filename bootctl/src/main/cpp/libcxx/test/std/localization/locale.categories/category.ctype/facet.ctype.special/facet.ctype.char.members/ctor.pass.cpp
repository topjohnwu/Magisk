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

// explicit ctype(const mask* tbl = 0, bool del = false, size_t refs = 0);

#include <locale>
#include <cassert>

class my_facet
    : public std::ctype<char>
{
public:
    static int count;

    explicit my_facet(const mask* tbl = 0, bool del = false, std::size_t refs = 0)
        : std::ctype<char>(tbl, del, refs) {++count;}

    ~my_facet() {--count;}
};

int my_facet::count = 0;

int main()
{
    {
        std::locale l(std::locale::classic(), new my_facet);
        assert(my_facet::count == 1);
    }
    assert(my_facet::count == 0);
    {
        my_facet f(0, false, 1);
        assert(my_facet::count == 1);
        {
            std::locale l(std::locale::classic(), &f);
            assert(my_facet::count == 1);
        }
        assert(my_facet::count == 1);
    }
    assert(my_facet::count == 0);
}
