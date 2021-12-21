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

// <mutex>

// template <class Mutex> class unique_lock;

// unique_lock(mutex_type& m, try_to_lock_t);

#include <mutex>
#include <thread>
#include <cstdlib>
#include <cassert>

std::mutex m;

typedef std::chrono::system_clock Clock;
typedef Clock::time_point time_point;
typedef Clock::duration duration;
typedef std::chrono::milliseconds ms;
typedef std::chrono::nanoseconds ns;

void f()
{
    time_point t0 = Clock::now();
    {
        std::unique_lock<std::mutex> lk(m, std::try_to_lock);
        assert(lk.owns_lock() == false);
    }
    {
        std::unique_lock<std::mutex> lk(m, std::try_to_lock);
        assert(lk.owns_lock() == false);
    }
    {
        std::unique_lock<std::mutex> lk(m, std::try_to_lock);
        assert(lk.owns_lock() == false);
    }
    while (true)
    {
        std::unique_lock<std::mutex> lk(m, std::try_to_lock);
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
    std::thread t(f);
    std::this_thread::sleep_for(ms(250));
    m.unlock();
    t.join();
}
