//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <> class codecvt<char16_t, char, mbstate_t>

// explicit codecvt(size_t refs = 0);

#include <locale>
#include <cassert>

//#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS

typedef std::codecvt<char16_t, char, std::mbstate_t> F;

class my_facet
    : public F
{
public:
    static int count;

    explicit my_facet(std::size_t refs = 0)
        : F(refs) {++count;}

    ~my_facet() {--count;}
};

int my_facet::count = 0;

//#endif

int main()
{
//#ifndef _LIBCPP_HAS_NO_UNICODE_CHARS
    {
        std::locale l(std::locale::classic(), new my_facet);
        assert(my_facet::count == 1);
    }
    assert(my_facet::count == 0);
    {
        my_facet f(1);
        assert(my_facet::count == 1);
        {
            std::locale l(std::locale::classic(), &f);
            assert(my_facet::count == 1);
        }
        assert(my_facet::count == 1);
    }
    assert(my_facet::count == 0);
//#endif
}
