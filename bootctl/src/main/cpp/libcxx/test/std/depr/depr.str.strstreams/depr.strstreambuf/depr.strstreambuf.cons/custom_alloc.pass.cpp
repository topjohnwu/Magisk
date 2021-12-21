//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <strstream>

// class strstreambuf

// strstreambuf(void* (*palloc_arg)(size_t), void (*pfree_arg)(void*));

#include <strstream>
#include <cassert>

int called = 0;

void* my_alloc(std::size_t)
{
    static char buf[10000];
    ++called;
    return buf;
}

void my_free(void*)
{
    ++called;
}

struct test
    : std::strstreambuf
{
    test(void* (*palloc_arg)(size_t), void (*pfree_arg)(void*))
        : std::strstreambuf(palloc_arg, pfree_arg) {}
    virtual int_type overflow(int_type c)
        {return std::strstreambuf::overflow(c);}
};

int main()
{
    {
        test s(my_alloc, my_free);
        assert(called == 0);
        s.overflow('a');
        assert(called == 1);
    }
    assert(called == 2);
}
