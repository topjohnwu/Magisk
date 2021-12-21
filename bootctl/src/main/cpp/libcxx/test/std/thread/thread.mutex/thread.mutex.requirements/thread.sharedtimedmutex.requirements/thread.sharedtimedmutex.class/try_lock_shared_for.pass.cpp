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
// UNSUPPORTED: c++98, c++03, c++11

// FLAKY_TEST.

// <shared_mutex>

// class shared_timed_mutex;

// template <class Rep, class Period>
//     bool try_lock_shared_for(const chrono::duration<Rep, Period>& rel_time);

#include <shared_mutex>
#include <thread>
#include <vector>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"

std::shared_timed_mutex m;

typedef std::chrono::steady_clock Clock;
typedef Clock::time_point time_point;
typedef Clock::duration duration;
typedef std::chrono::milliseconds ms;
typedef std::chrono::nanoseconds ns;

ms WaitTime = ms(250);

// Thread sanitizer causes more overhead and will sometimes cause this test
// to fail. To prevent this we give Thread sanitizer more time to complete the
// test.
#if !defined(TEST_HAS_SANITIZERS)
ms Tolerance = ms(50);
#else
ms Tolerance = ms(50 * 5);
#endif

void f1()
{
    time_point t0 = Clock::now();
    assert(m.try_lock_shared_for(WaitTime + Tolerance) == true);
    time_point t1 = Clock::now();
    m.unlock_shared();
    ns d = t1 - t0 - WaitTime;
    assert(d < Tolerance);  // within 50ms
}

void f2()
{
    time_point t0 = Clock::now();
    assert(m.try_lock_shared_for(WaitTime) == false);
    time_point t1 = Clock::now();
    ns d = t1 - t0 - WaitTime;
    assert(d < Tolerance);  // within 50ms
}

int main()
{
    {
        m.lock();
        std::vector<std::thread> v;
        for (int i = 0; i < 5; ++i)
            v.push_back(std::thread(f1));
        std::this_thread::sleep_for(WaitTime);
        m.unlock();
        for (auto& t : v)
            t.join();
    }
    {
        m.lock();
        std::vector<std::thread> v;
        for (int i = 0; i < 5; ++i)
            v.push_back(std::thread(f2));
        std::this_thread::sleep_for(WaitTime + Tolerance);
        m.unlock();
        for (auto& t : v)
            t.join();
    }
}
