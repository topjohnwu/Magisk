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

// Addition

#include <ios>
#include <cassert>

int main()
{
    typedef std::fpos<std::mbstate_t> P;
    P p(5);
    std::streamoff o(6);
    P q = p + o;
    assert(q == P(11));
    p += o;
    assert(p == q);
}
