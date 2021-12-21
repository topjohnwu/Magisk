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

// FLAKY_TEST

// <shared_mutex>

// template <class Mutex> class shared_lock;

// shared_lock(mutex_type& m, try_to_lock_t);

#include <shared_mutex>
#include <thread>
#include <vector>
#include <cstdlib>
#include <cassert>

std::shared_timed_mutex m;

typedef std::chrono::system_clock Clock;
typedef Clock::time_point time_point;
typedef Clock::duration duration;
typedef std::chrono::milliseconds ms;
typedef std::chrono::nanoseconds ns;

void f()
{
    time_point t0 = Clock::now();
    {
        std::shared_lock<std::shared_timed_mutex> lk(m, std::try_to_lock);
        assert(lk.owns_lock() == false);
    }
    {
        std::shared_lock<std::shared_timed_mutex> lk(m, std::try_to_lock);
        assert(lk.owns_lock() == false);
    }
    {
        std::shared_lock<std::shared_timed_mutex> lk(m, std::try_to_lock);
        assert(lk.owns_lock() == false);
    }
    while (true)
    {
        std::shared_lock<std::shared_timed_mutex> lk(m, std::try_to_lock);
        if (lk.owns_lock())
            break;
    }
    time_point t1 = Clock::now();
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
