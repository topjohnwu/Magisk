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

// template <class Rep, class Period>
//   future_status
//   wait_for(const chrono::duration<Rep, Period>& rel_time) const;

#include <future>
#include <cassert>

typedef std::chrono::milliseconds ms;

void func1(std::promise<int> p)
{
    std::this_thread::sleep_for(ms(500));
    p.set_value(3);
}

int j = 0;

void func3(std::promise<int&> p)
{
    std::this_thread::sleep_for(ms(500));
    j = 5;
    p.set_value(j);
}

void func5(std::promise<void> p)
{
    std::this_thread::sleep_for(ms(500));
    p.set_value();
}

int main()
{
    typedef std::chrono::high_resolution_clock Clock;
    {
        typedef int T;
        std::promise<T> p;
        std::shared_future<T> f = p.get_future();
        std::thread(func1, std::move(p)).detach();
        assert(f.valid());
        assert(f.wait_for(ms(300)) == std::future_status::timeout);
        assert(f.valid());
        assert(f.wait_for(ms(300)) == std::future_status::ready);
        assert(f.valid());
        Clock::time_point t0 = Clock::now();
        f.wait();
        Clock::time_point t1 = Clock::now();
        assert(f.valid());
        assert(t1-t0 < ms(5));
    }
    {
        typedef int& T;
        std::promise<T> p;
        std::shared_future<T> f = p.get_future();
        std::thread(func3, std::move(p)).detach();
        assert(f.valid());
        assert(f.wait_for(ms(300)) == std::future_status::timeout);
        assert(f.valid());
        assert(f.wait_for(ms(300)) == std::future_status::ready);
        assert(f.valid());
        Clock::time_point t0 = Clock::now();
        f.wait();
        Clock::time_point t1 = Clock::now();
        assert(f.valid());
        assert(t1-t0 < ms(5));
    }
    {
        typedef void T;
        std::promise<T> p;
        std::shared_future<T> f = p.get_future();
        std::thread(func5, std::move(p)).detach();
        assert(f.valid());
        assert(f.wait_for(ms(300)) == std::future_status::timeout);
        assert(f.valid());
        assert(f.wait_for(ms(300)) == std::future_status::ready);
        assert(f.valid());
        Clock::time_point t0 = Clock::now();
        f.wait();
        Clock::time_point t1 = Clock::now();
        assert(f.valid());
        assert(t1-t0 < ms(5));
    }
}
