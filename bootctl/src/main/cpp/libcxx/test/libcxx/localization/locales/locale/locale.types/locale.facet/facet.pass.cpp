//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// class locale::facet
// {
// protected:
//     explicit facet(size_t refs = 0);
//     virtual ~facet();
//     facet(const facet&) = delete;
//     void operator=(const facet&) = delete;
// };

// This test isn't portable

#include <locale>
#include <cassert>

struct my_facet
    : public std::locale::facet
{
    static int count;
    my_facet(unsigned refs = 0)
        : std::locale::facet(refs)
        {++count;}

    ~my_facet() {--count;}
};

int my_facet::count = 0;

int main()
{
    my_facet* f = new my_facet;
    f->__add_shared();
    assert(my_facet::count == 1);
    f->__release_shared();
    assert(my_facet::count == 0);
    f = new my_facet(1);
    f->__add_shared();
    assert(my_facet::count == 1);
    f->__release_shared();
    assert(my_facet::count == 1);
    f->__release_shared();
    assert(my_facet::count == 0);
}
