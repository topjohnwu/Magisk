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

// bool operator=(const bernoulli_distribution& x,
//                const bernoulli_distribution& y);
// bool operator!(const bernoulli_distribution& x,
//                const bernoulli_distribution& y);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::bernoulli_distribution D;
        D d1(.25);
        D d2(.25);
        assert(d1 == d2);
    }
    {
        typedef std::bernoulli_distribution D;
        D d1(.28);
        D d2(.25);
        assert(d1 != d2);
    }
}
