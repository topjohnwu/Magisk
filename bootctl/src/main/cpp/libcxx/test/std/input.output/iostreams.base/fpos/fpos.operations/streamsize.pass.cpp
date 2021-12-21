//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ios>

// streamsize and streamoff interconvert

#include <ios>
#include <cassert>

int main()
{
    std::streamoff o(5);
    std::streamsize sz(o);
    assert(sz == 5);
    std::streamoff o2(sz);
    assert(o == o2);
}
