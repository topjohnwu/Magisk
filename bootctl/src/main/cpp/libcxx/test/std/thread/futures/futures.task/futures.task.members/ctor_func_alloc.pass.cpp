//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads
// UNSUPPORTED: c++98, c++03
// REQUIRES: c++11 || c++14
// packaged_task allocator support was removed in C++17 (LWG 2921)

// <future>

// class packaged_task<R(ArgTypes...)>

// template <class F, class Allocator>
//     explicit packaged_task(allocator_arg_t, const Allocator& a, F&& f);

#include <future>
#include <cassert>

#include "test_allocator.h"
#include "min_allocator.h"

class A
{
    long data_;

public:
    static int n_moves;
    static int n_copies;

    explicit A(long i) : data_(i) {}
    A(A&& a) : data_(a.data_) {++n_moves; a.data_ = -1;}
    A(const A& a) : data_(a.data_) {++n_copies;}

    long operator()(long i, long j) const {return data_ + i + j;}
};

int A::n_moves = 0;
int A::n_copies = 0;

int func(int i) { return i; }

int main()
{
    {
        std::packaged_task<double(int, char)> p(std::allocator_arg,
                                                test_allocator<A>(), A(5));
        assert(test_alloc_base::alloc_count > 0);
        assert(p.valid());
        std::future<double> f = p.get_future();
        p(3, 'a');
        assert(f.get() == 105.0);
        assert(A::n_copies == 0);
        assert(A::n_moves > 0);
    }
    assert(test_alloc_base::alloc_count == 0);
    A::n_copies = 0;
    A::n_moves  = 0;
    {
        A a(5);
        std::packaged_task<double(int, char)> p(std::allocator_arg,
                                                test_allocator<A>(), a);
        assert(test_alloc_base::alloc_count > 0);
        assert(p.valid());
        std::future<double> f = p.get_future();
        p(3, 'a');
        assert(f.get() == 105.0);
        assert(A::n_copies > 0);
        assert(A::n_moves >= 0);
    }
    assert(test_alloc_base::alloc_count == 0);
    A::n_copies = 0;
    A::n_moves  = 0;
    {
        A a(5);
        std::packaged_task<int(int)> p(std::allocator_arg, test_allocator<A>(), &func);
        assert(test_alloc_base::alloc_count > 0);
        assert(p.valid());
        std::future<int> f = p.get_future();
        p(4);
        assert(f.get() == 4);
    }
    assert(test_alloc_base::alloc_count == 0);
    A::n_copies = 0;
    A::n_moves  = 0;
    {
        A a(5);
        std::packaged_task<int(int)> p(std::allocator_arg, test_allocator<A>(), func);
        assert(test_alloc_base::alloc_count > 0);
        assert(p.valid());
        std::future<int> f = p.get_future();
        p(4);
        assert(f.get() == 4);
    }
    assert(test_alloc_base::alloc_count == 0);
    A::n_copies = 0;
    A::n_moves  = 0;
    {
        std::packaged_task<double(int, char)> p(std::allocator_arg,
                                                bare_allocator<void>(), A(5));
        assert(p.valid());
        std::future<double> f = p.get_future();
        p(3, 'a');
        assert(f.get() == 105.0);
        assert(A::n_copies == 0);
        assert(A::n_moves > 0);
    }
    A::n_copies = 0;
    A::n_moves  = 0;
    {
        std::packaged_task<double(int, char)> p(std::allocator_arg,
                                                min_allocator<void>(), A(5));
        assert(p.valid());
        std::future<double> f = p.get_future();
        p(3, 'a');
        assert(f.get() == 105.0);
        assert(A::n_copies == 0);
        assert(A::n_moves > 0);
    }
    A::n_copies = 0;
    A::n_moves  = 0;
}
