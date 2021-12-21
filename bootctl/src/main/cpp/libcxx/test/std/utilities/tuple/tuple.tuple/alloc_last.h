//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef ALLOC_LAST_H
#define ALLOC_LAST_H

#include <cassert>

#include "allocators.h"

struct alloc_last
{
    static bool allocator_constructed;

    typedef A1<int> allocator_type;

    int data_;

    alloc_last() : data_(0) {}
    alloc_last(int d) : data_(d) {}
    alloc_last(const A1<int>& a)
        : data_(0)
    {
        assert(a.id() == 5);
        allocator_constructed = true;
    }

    alloc_last(int d, const A1<int>& a)
        : data_(d)
    {
        assert(a.id() == 5);
        allocator_constructed = true;
    }

    alloc_last(const alloc_last& d, const A1<int>& a)
        : data_(d.data_)
    {
        assert(a.id() == 5);
        allocator_constructed = true;
    }

    ~alloc_last() {data_ = -1;}

    friend bool operator==(const alloc_last& x, const alloc_last& y)
        {return x.data_ == y.data_;}
    friend bool operator< (const alloc_last& x, const alloc_last& y)
        {return x.data_ < y.data_;}
};

bool alloc_last::allocator_constructed = false;

#endif  // ALLOC_LAST_H
