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

// explicit ctype(size_t refs = 0);

#include <locale>
#include <cassert>

template <class C>
class my_facet
    : public std::ctype<C>
{
public:
    static int count;

    explicit my_facet(std::size_t refs = 0)
        : std::ctype<C>(refs) {++count;}

    ~my_facet() {--count;}
};

template <class C> int my_facet<C>::count = 0;

int main()
{
    {
        std::locale l(std::locale::classic(), new my_facet<wchar_t>);
        assert(my_facet<wchar_t>::count == 1);
    }
    assert(my_facet<wchar_t>::count == 0);
    {
        my_facet<wchar_t> f(1);
        assert(my_facet<wchar_t>::count == 1);
        {
            std::locale l(std::locale::classic(), &f);
            assert(my_facet<wchar_t>::count == 1);
        }
        assert(my_facet<wchar_t>::count == 1);
    }
    assert(my_facet<wchar_t>::count == 0);
}
