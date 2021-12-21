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

// FLAKY_TEST

// <mutex>

// template <class Mutex> class unique_lock;

// void lock();

#include <mutex>
#include <thread>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"

std::mutex m;

typedef std::chrono::system_clock Clock;
typedef Clock::time_point time_point;
typedef Clock::duration duration;
typedef std::chrono::milliseconds ms;
typedef std::chrono::nanoseconds ns;

void f()
{
    std::unique_lock<std::mutex> lk(m, std::defer_lock);
    time_point t0 = Clock::now();
    lk.lock();
    time_point t1 = Clock::now();
    assert(lk.owns_lock() == true);
    ns d = t1 - t0 - ms(250);
    assert(d < ms(25));  // within 25ms
#ifndef TEST_HAS_NO_EXCEPTIONS
    try
    {
        lk.lock();
        assert(false);
    }
    catch (std::system_error& e)
    {
        assert(e.code().value() == EDEADLK);
    }
#endif
    lk.unlock();
    lk.release();
#ifndef TEST_HAS_NO_EXCEPTIONS
    try
    {
        lk.lock();
        assert(false);
    }
    catch (std::system_error& e)
    {
        assert(e.code().value() == EPERM);
    }
#endif
}

int main()
{
    m.lock();
    std::thread t(f);
    std::this_thread::sleep_for(ms(250));
    m.unlock();
    t.join();
}
