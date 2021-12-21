//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <tuple>

// template <class... Types> class tuple;

// tuple& operator=(const tuple& u);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <cassert>

#include "MoveOnly.h"

int main()
{
    {
        typedef std::tuple<MoveOnly> T;
        T t0(MoveOnly(2));
        T t;
        t = t0;
    }
}
