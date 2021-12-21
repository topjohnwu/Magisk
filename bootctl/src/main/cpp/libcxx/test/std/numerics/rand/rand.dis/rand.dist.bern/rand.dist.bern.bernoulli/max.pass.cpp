//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// class bernoulli_distribution

// result_type max() const;

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::bernoulli_distribution D;
        D d(.25);
        assert(d.max() == true);
    }
}
