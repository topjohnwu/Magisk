//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class RealType, size_t bits, class URNG>
//     RealType generate_canonical(URNG& g);

#include <random>
#include <cassert>

#include "truncate_fp.h"

int main()
{
    {
        typedef std::minstd_rand0 E;
        typedef float F;
        E r;
        F f = std::generate_canonical<F, 0>(r);
        assert(f == truncate_fp((16807 - E::min()) / (E::max() - E::min() + F(1))));
    }
    {
        typedef std::minstd_rand0 E;
        typedef float F;
        E r;
        F f = std::generate_canonical<F, 1>(r);
        assert(f == truncate_fp((16807 - E::min()) / (E::max() - E::min() + F(1))));
    }
    {
        typedef std::minstd_rand0 E;
        typedef float F;
        E r;
        F f = std::generate_canonical<F, std::numeric_limits<F>::digits - 1>(r);
        assert(f == truncate_fp((16807 - E::min()) / (E::max() - E::min() + F(1))));
    }
    {
        typedef std::minstd_rand0 E;
        typedef float F;
        E r;
        F f = std::generate_canonical<F, std::numeric_limits<F>::digits>(r);
        assert(f == truncate_fp((16807 - E::min()) / (E::max() - E::min() + F(1))));
    }
    {
        typedef std::minstd_rand0 E;
        typedef float F;
        E r;
        F f = std::generate_canonical<F, std::numeric_limits<F>::digits + 1>(r);
        assert(f == truncate_fp((16807 - E::min()) / (E::max() - E::min() + F(1))));
    }

    {
        typedef std::minstd_rand0 E;
        typedef double F;
        E r;
        F f = std::generate_canonical<F, 0>(r);
        assert(f == truncate_fp((16807 - E::min()) / (E::max() - E::min() + F(1))));
    }
    {
        typedef std::minstd_rand0 E;
        typedef double F;
        E r;
        F f = std::generate_canonical<F, 1>(r);
        assert(f == truncate_fp((16807 - E::min()) / (E::max() - E::min() + F(1))));
    }
    {
        typedef std::minstd_rand0 E;
        typedef double F;
        E r;
        F f = std::generate_canonical<F, std::numeric_limits<F>::digits - 1>(r);
        assert(f == truncate_fp(
            (16807 - E::min() +
            (282475249 - E::min()) * (E::max() - E::min() + F(1))) /
            ((E::max() - E::min() + F(1)) * (E::max() - E::min() + F(1)))));
    }
    {
        typedef std::minstd_rand0 E;
        typedef double F;
        E r;
        F f = std::generate_canonical<F, std::numeric_limits<F>::digits>(r);
        assert(f == truncate_fp(
            (16807 - E::min() +
            (282475249 - E::min()) * (E::max() - E::min() + F(1))) /
            ((E::max() - E::min() + F(1)) * (E::max() - E::min() + F(1)))));
    }
    {
        typedef std::minstd_rand0 E;
        typedef double F;
        E r;
        F f = std::generate_canonical<F, std::numeric_limits<F>::digits + 1>(r);
        assert(f == truncate_fp(
            (16807 - E::min() +
            (282475249 - E::min()) * (E::max() - E::min() + F(1))) /
            ((E::max() - E::min() + F(1)) * (E::max() - E::min() + F(1)))));
    }
}
