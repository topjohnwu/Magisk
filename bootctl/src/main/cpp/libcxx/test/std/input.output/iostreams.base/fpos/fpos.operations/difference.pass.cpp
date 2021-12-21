//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <ios>

// template <class StateT> class fpos

// Subraction with fpos

#include <ios>
#include <cassert>

int main()
{
    typedef std::fpos<std::mbstate_t> P;
    P p(11);
    P q(6);
    std::streamoff o = p - q;
    assert(o == 5);
}
