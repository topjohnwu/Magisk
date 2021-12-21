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

// <future>

// class packaged_task<R(ArgTypes...)>

// ~packaged_task();

#include <future>
#include <cassert>

#include "test_macros.h"

class A
{
    long data_;

public:
    explicit A(long i) : data_(i) {}

    long operator()(long i, long j) const {return data_ + i + j;}
};

void func(std::packaged_task<double(int, char)>)
{
}

void func2(std::packaged_task<double(int, char)> p)
{
    p(3, 'a');
}

int main()
{
#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        std::packaged_task<double(int, char)> p(A(5));
        std::future<double> f = p.get_future();
        std::thread(func, std::move(p)).detach();
        try
        {
            double i = f.get();
            ((void)i); // Prevent unused warning
            assert(false);
        }
        catch (const std::future_error& e)
        {
            assert(e.code() == make_error_code(std::future_errc::broken_promise));
        }
    }
#endif
    {
        std::packaged_task<double(int, char)> p(A(5));
        std::future<double> f = p.get_future();
        std::thread(func2, std::move(p)).detach();
        assert(f.get() == 105.0);
    }
}
