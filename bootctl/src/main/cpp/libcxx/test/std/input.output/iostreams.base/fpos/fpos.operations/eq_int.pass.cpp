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

// == and !=

#include <ios>
#include <cassert>

int main()
{
    typedef std::fpos<std::mbstate_t> P;
    P p(5);
    P q(6);
    assert(p == p);
    assert(p != q);
}
