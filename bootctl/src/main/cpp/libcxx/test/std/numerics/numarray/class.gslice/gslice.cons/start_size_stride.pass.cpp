//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <valarray>

// class glice;

// gslice(size_t start, const valarray<size_t>& size,
//                      const valarray<size_t>& stride);

#include <valarray>
#include <cassert>

int main()
{
    std::size_t a1[] = {1, 2, 3};
    std::size_t a2[] = {4, 5, 6};
    std::valarray<std::size_t> size(a1, sizeof(a1)/sizeof(a1[0]));
    std::valarray<std::size_t> stride(a2, sizeof(a2)/sizeof(a2[0]));
    std::gslice gs(7, size, stride);
    assert(gs.start() == 7);
    std::valarray<std::size_t> r = gs.size();
    assert(r.size() == 3);
    assert(r[0] == 1);
    assert(r[1] == 2);
    assert(r[2] == 3);
    r = gs.stride();
    assert(r.size() == 3);
    assert(r[0] == 4);
    assert(r[1] == 5);
    assert(r[2] == 6);
}
