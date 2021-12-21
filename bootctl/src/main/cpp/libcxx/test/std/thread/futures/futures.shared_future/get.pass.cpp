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

// class shared_future<R>

// const R& shared_future::get();
// R& shared_future<R&>::get();
// void shared_future<void>::get();

#include <future>
#include <cassert>

#include "test_macros.h"

void func1(std::promise<int> p)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    p.set_value(3);
}

void func2(std::promise<int> p)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    p.set_exception(std::make_exception_ptr(3));
}

int j = 0;

void func3(std::promise<int&> p)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    j = 5;
    p.set_value(j);
}

void func4(std::promise<int&> p)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    p.set_exception(std::make_exception_ptr(3.5));
}

void func5(std::promise<void> p)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    p.set_value();
}

void func6(std::promise<void> p)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    p.set_exception(std::make_exception_ptr('c'));
}

int main()
{
    {
        typedef int T;
        {
            std::promise<T> p;
            std::shared_future<T> f = p.get_future();
            std::thread(func1, std::move(p)).detach();
            assert(f.valid());
            assert(f.get() == 3);
            assert(f.valid());
        }
#ifndef TEST_HAS_NO_EXCEPTIONS
        {
            std::promise<T> p;
            std::shared_future<T> f = p.get_future();
            std::thread(func2, std::move(p)).detach();
            try
            {
                assert(f.valid());
                assert(f.get() == 3);
                assert(false);
            }
            catch (int i)
            {
                assert(i == 3);
            }
            assert(f.valid());
        }
#endif
    }
    {
        typedef int& T;
        {
            std::promise<T> p;
            std::shared_future<T> f = p.get_future();
            std::thread(func3, std::move(p)).detach();
            assert(f.valid());
            assert(f.get() == 5);
            assert(f.valid());
        }
#ifndef TEST_HAS_NO_EXCEPTIONS
        {
            std::promise<T> p;
            std::shared_future<T> f = p.get_future();
            std::thread(func4, std::move(p)).detach();
            try
            {
                assert(f.valid());
                assert(f.get() == 3);
                assert(false);
            }
            catch (double i)
            {
                assert(i == 3.5);
            }
            assert(f.valid());
        }
#endif
    }
    {
        typedef void T;
        {
            std::promise<T> p;
            std::shared_future<T> f = p.get_future();
            std::thread(func5, std::move(p)).detach();
            assert(f.valid());
            f.get();
            assert(f.valid());
        }
#ifndef TEST_HAS_NO_EXCEPTIONS
        {
            std::promise<T> p;
            std::shared_future<T> f = p.get_future();
            std::thread(func6, std::move(p)).detach();
            try
            {
                assert(f.valid());
                f.get();
                assert(false);
            }
            catch (char i)
            {
                assert(i == 'c');
            }
            assert(f.valid());
        }
#endif
    }
}
