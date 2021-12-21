//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef ALLOC_FIRST_H
#define ALLOC_FIRST_H

#include <cassert>

#include "allocators.h"

struct alloc_first
{
    static bool allocator_constructed;

    typedef A1<int> allocator_type;

    int data_;

    alloc_first() : data_(0) {}
    alloc_first(int d) : data_(d) {}
    alloc_first(std::allocator_arg_t, const A1<int>& a)
        : data_(0)
    {
        assert(a.id() == 5);
        allocator_constructed = true;
    }

    alloc_first(std::allocator_arg_t, const A1<int>& a, int d)
        : data_(d)
    {
        assert(a.id() == 5);
        allocator_constructed = true;
    }

    alloc_first(std::allocator_arg_t, const A1<int>& a, const alloc_first& d)
        : data_(d.data_)
    {
        assert(a.id() == 5);
        allocator_constructed = true;
    }

    ~alloc_first() {data_ = -1;}

    friend bool operator==(const alloc_first& x, const alloc_first& y)
        {return x.data_ == y.data_;}
    friend bool operator< (const alloc_first& x, const alloc_first& y)
        {return x.data_ < y.data_;}
};

bool alloc_first::allocator_constructed = false;

#endif  // ALLOC_FIRST_H
