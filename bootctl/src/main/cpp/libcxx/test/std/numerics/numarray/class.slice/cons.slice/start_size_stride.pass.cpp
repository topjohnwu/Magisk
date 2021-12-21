//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <valarray>

// class slice;

// slice(size_t start, size_t size, size_t stride);

#include <valarray>
#include <cassert>

int main()
{
    std::slice s(1, 3, 2);
    assert(s.start() == 1);
    assert(s.size() == 3);
    assert(s.stride() == 2);
}
