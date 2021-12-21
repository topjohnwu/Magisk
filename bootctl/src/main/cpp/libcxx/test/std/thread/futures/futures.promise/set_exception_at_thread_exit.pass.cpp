//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-no-exceptions
// UNSUPPORTED: libcpp-has-no-threads
// UNSUPPORTED: c++98, c++03

// <future>

// class promise<R>

// void promise::set_exception_at_thread_exit(exception_ptr p);

#include <future>
#include <cassert>

void func(std::promise<int> p)
{
    p.set_exception_at_thread_exit(std::make_exception_ptr(3));
}

int main()
{
    {
        typedef int T;
        std::promise<T> p;
        std::future<T> f = p.get_future();
        std::thread(func, std::move(p)).detach();
        try
        {
            f.get();
            assert(false);
        }
        catch (int i)
        {
            assert(i == 3);
        }
    }
}
