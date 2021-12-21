//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads, c++98, c++03

// <future>

// class promise<R>

// void promise::set_value_at_thread_exit(R&& r);

#include <future>
#include <memory>
#include <cassert>

void func(std::promise<std::unique_ptr<int>> p)
{
    p.set_value_at_thread_exit(std::unique_ptr<int>(new int(5)));
}

int main()
{
    {
        std::promise<std::unique_ptr<int>> p;
        std::future<std::unique_ptr<int>> f = p.get_future();
        std::thread(func, std::move(p)).detach();
        assert(*f.get() == 5);
    }
}
