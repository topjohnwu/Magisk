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

// bool try_lock_shared();

#include <shared_mutex>
#include <thread>
#include <vector>
#include <cstdlib>
#include <cassert>

std::shared_mutex m;

typedef std::chrono::system_clock Clock;
typedef Clock::time_point time_point;
typedef Clock::duration duration;
typedef std::chrono::milliseconds ms;
typedef std::chrono::nanoseconds ns;

void f()
{
    time_point t0 = Clock::now();
    assert(!m.try_lock_shared());
    assert(!m.try_lock_shared());
    assert(!m.try_lock_shared());
    while(!m.try_lock_shared())
        ;
    time_point t1 = Clock::now();
    m.unlock_shared();
    ns d = t1 - t0 - ms(250);
    assert(d < ms(200));  // within 200ms
}


int main()
{
    m.lock();
    std::vector<std::thread> v;
    for (int i = 0; i < 5; ++i)
        v.push_back(std::thread(f));
    std::this_thread::sleep_for(ms(250));
    m.unlock();
    for (auto& t : v)
        t.join();
}
