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

// template <class Mutex> class shared_lock;

// explicit shared_lock(mutex_type& m);

// template<class _Mutex> shared_lock(shared_lock<_Mutex>)
//     -> shared_lock<_Mutex>;  // C++17

#include <shared_mutex>
#include <thread>
#include <vector>
#include <cstdlib>
#include <cassert>

#include "test_macros.h"

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

std::shared_timed_mutex m;

void f()
{
    time_point t0 = Clock::now();
    time_point t1;
    {
    std::shared_lock<std::shared_timed_mutex> ul(m);
    t1 = Clock::now();
    }
    ns d = t1 - t0 - WaitTime;
    assert(d < Tolerance);  // within tolerance
}

void g()
{
    time_point t0 = Clock::now();
    time_point t1;
    {
    std::shared_lock<std::shared_timed_mutex> ul(m);
    t1 = Clock::now();
    }
    ns d = t1 - t0;
    assert(d < Tolerance);  // within tolerance
}

int main()
{
    std::vector<std::thread> v;
    {
        m.lock();
        for (int i = 0; i < 5; ++i)
            v.push_back(std::thread(f));
        std::this_thread::sleep_for(WaitTime);
        m.unlock();
        for (auto& t : v)
            t.join();
    }
    {
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

#ifdef __cpp_deduction_guides
    std::shared_lock sl(m);
    static_assert((std::is_same<decltype(sl), std::shared_lock<decltype(m)>>::value), "" );
#endif
}
