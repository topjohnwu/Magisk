//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class Engine, size_t k>
// class shuffle_order_engine

// explicit shuffle_order_engine(const Engine& e);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::minstd_rand0 Engine;
        typedef std::knuth_b Adaptor;
        Engine e;
        Adaptor a(e);
        for (unsigned k = 0; k <= Adaptor::table_size; ++k) {
            (void)e();
        }

        assert(a.base() == e);
    }
}
