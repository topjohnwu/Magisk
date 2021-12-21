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

// class promise<R>

// void promise::set_value_at_thread_exit(const R& r);

#include <future>
#include <cassert>

void func(std::promise<int> p)
{
    const int i = 5;
    p.set_value_at_thread_exit(i);
}

int main()
{
    {
        std::promise<int> p;
        std::future<int> f = p.get_future();
        std::thread(func, std::move(p)).detach();
        assert(f.get() == 5);
    }
}
