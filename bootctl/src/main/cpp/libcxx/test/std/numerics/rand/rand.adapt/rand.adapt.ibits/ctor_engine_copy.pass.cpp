//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <random>

// template<class Engine, size_t w, class UIntType>
// class independent_bits_engine

// explicit independent_bits_engine(const Engine& e);

#include <random>
#include <cassert>

int main()
{
    {
        typedef std::mt19937 Engine;
        typedef std::independent_bits_engine<Engine, 24, unsigned> Adaptor;
        Engine e;
        Adaptor a(e);
        assert(a.base() == e);
    }
}
