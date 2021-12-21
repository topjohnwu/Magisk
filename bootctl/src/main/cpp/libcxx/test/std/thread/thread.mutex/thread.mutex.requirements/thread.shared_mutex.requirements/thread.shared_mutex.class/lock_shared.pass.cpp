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
// UNSUPPORTED: c++98, c++03, c++11, c++14

// FLAKY_TEST.

// <shared_mutex>

// class shared_mutex;

// void lock_shared();

#include <shared_mutex>
#include <thread>
#include <vector>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"

std::shared_mutex m;

typedef std::chrono::system_clock Clock;
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

void f()
{
    time_point t0 = Clock::now();
    m.lock_shared();
    time_point t1 = Clock::now();
    m.unlock_shared();
    ns d = t1 - t0 - WaitTime;
    assert(d < Tolerance);  // within tolerance
}

void g()
{
    time_point t0 = Clock::now();
    m.lock_shared();
    time_point t1 = Clock::now();
    m.unlock_shared();
    ns d = t1 - t0;
    assert(d < Tolerance);  // within tolerance
}


int main()
{
    m.lock();
    std::vector<std::thread> v;
    for (int i = 0; i < 5; ++i)
        v.push_back(std::thread(f));
    std::this_thread::sleep_for(WaitTime);
    m.unlock();
    for (auto& t : v)
        t.join();
    m.lock_shared();
    for (auto& t : v)
        t = std::thread(g);
    std::thread q(f);
    std::this_thread::sleep_for(WaitTime);
    m.unlock_shared();
    for (auto& t : v)
        t.join();
    q.join();
}
