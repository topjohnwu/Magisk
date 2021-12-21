//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// shared_ptr

// template<class T, class A, class... Args>
//    shared_ptr<T> allocate_shared(const A& a, Args&&... args);


#include <memory>
#include <new>
#include <cstdlib>
#include <cassert>
#include "test_allocator.h"
#include "min_allocator.h"

struct Zero
{
    static int count;
    Zero() {++count;}
    Zero(Zero const &) {++count;}
    ~Zero() {--count;}
};

int Zero::count = 0;

struct One
{
    static int count;
    int value;
    explicit One(int v) : value(v) {++count;}
    One(One const & o) : value(o.value) {++count;}
    ~One() {--count;}
};

int One::count = 0;


struct Two
{
    static int count;
    int value;
    Two(int v, int) : value(v) {++count;}
    Two(Two const & o) : value(o.value) {++count;}
    ~Two() {--count;}
};

int Two::count = 0;

struct Three
{
    static int count;
    int value;
    Three(int v, int, int) : value(v) {++count;}
    Three(Three const & o) : value(o.value) {++count;}
    ~Three() {--count;}
};

int Three::count = 0;

template <class Alloc>
void test()
{
    int const bad = -1;
    {
    std::shared_ptr<Zero> p = std::allocate_shared<Zero>(Alloc());
    assert(Zero::count == 1);
    }
    assert(Zero::count == 0);
    {
    int const i = 42;
    std::shared_ptr<One> p = std::allocate_shared<One>(Alloc(), i);
    assert(One::count == 1);
    assert(p->value == i);
    }
    assert(One::count == 0);
    {
    int const i = 42;
    std::shared_ptr<Two> p = std::allocate_shared<Two>(Alloc(), i, bad);
    assert(Two::count == 1);
    assert(p->value == i);
    }
    assert(Two::count == 0);
    {
    int const i = 42;
    std::shared_ptr<Three> p = std::allocate_shared<Three>(Alloc(), i, bad, bad);
    assert(Three::count == 1);
    assert(p->value == i);
    }
    assert(Three::count == 0);
}

int main()
{
    {
    int i = 67;
    int const bad = -1;
    std::shared_ptr<Two> p = std::allocate_shared<Two>(test_allocator<Two>(54), i, bad);
    assert(test_allocator<Two>::alloc_count == 1);
    assert(Two::count == 1);
    assert(p->value == 67);
    }
    assert(Two::count == 0);
    assert(test_allocator<Two>::alloc_count == 0);

    test<bare_allocator<void> >();
#if TEST_STD_VER >= 11
    test<min_allocator<void> >();
#endif
}
