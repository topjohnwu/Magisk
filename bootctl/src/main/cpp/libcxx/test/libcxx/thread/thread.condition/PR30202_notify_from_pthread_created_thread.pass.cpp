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
// REQUIRES: libcpp-has-thread-api-pthread

// notify_all_at_thread_exit(...) requires move semantics to transfer the
// unique_lock.
// UNSUPPORTED: c++98, c++03

// <condition_variable>

// void
//   notify_all_at_thread_exit(condition_variable& cond, unique_lock<mutex> lk);

// Test that this function works with threads that were not created by
// std::thread. See: https://bugs.llvm.org/show_bug.cgi?id=30202


#include <condition_variable>
#include <mutex>
#include <thread>
#include <chrono>
#include <cassert>
#include <pthread.h>

std::condition_variable cv;
std::mutex mut;
bool exited = false;

typedef std::chrono::milliseconds ms;
typedef std::chrono::high_resolution_clock Clock;

void* func(void*)
{
    std::unique_lock<std::mutex> lk(mut);
    std::notify_all_at_thread_exit(cv, std::move(lk));
    std::this_thread::sleep_for(ms(300));
    exited = true;
    return nullptr;
}

int main()
{
    {
    std::unique_lock<std::mutex> lk(mut);
    pthread_t id;
    int res = pthread_create(&id, 0, &func, nullptr);
    assert(res == 0);
    Clock::time_point t0 = Clock::now();
    assert(exited == false);
    cv.wait(lk);
    Clock::time_point t1 = Clock::now();
    assert(exited);
    assert(t1-t0 > ms(250));
    pthread_join(id, 0);
    }
    exited = false;
    {
    std::unique_lock<std::mutex> lk(mut);
    std::thread t(&func, nullptr);
    Clock::time_point t0 = Clock::now();
    assert(exited == false);
    cv.wait(lk);
    Clock::time_point t1 = Clock::now();
    assert(exited);
    assert(t1-t0 > ms(250));
    t.join();
    }
}
