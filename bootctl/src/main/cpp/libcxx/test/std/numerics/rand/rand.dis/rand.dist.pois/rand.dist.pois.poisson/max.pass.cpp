//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class IntType = int>
// class poisson_distribution

// result_type max() const;

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::poisson_distribution<> D;
        D d(.25);
        D::result_type m = d.max();
        assert(m == std::numeric_limits<int>::max());
    }
}
