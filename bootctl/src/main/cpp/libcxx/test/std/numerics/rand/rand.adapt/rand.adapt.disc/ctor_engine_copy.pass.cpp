//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class Engine, size_t p, size_t r>
// class discard_block_engine

// explicit discard_block_engine(const Engine& e);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::ranlux24_base Engine;
        typedef std::ranlux24 Adaptor;
        Engine e;
        Adaptor a(e);
        assert(a.base() == e);
    }
}
