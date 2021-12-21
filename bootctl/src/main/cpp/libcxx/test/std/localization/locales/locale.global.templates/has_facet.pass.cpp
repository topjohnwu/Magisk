//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <locale>

// template <class Facet> bool has_facet(const locale& loc) throw();

#include <locale>
#include <cassert>

struct my_facet
    : public std::locale::facet
{
    static std::locale::id id;
};

std::locale::id my_facet::id;

int main()
{
    std::locale loc;
    assert(std::has_facet<std::ctype<char> >(loc));
    assert(!std::has_facet<my_facet>(loc));
    std::locale loc2(loc, new my_facet);
    assert(std::has_facet<my_facet>(loc2));
}
