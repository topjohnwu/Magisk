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

// <condition_variable>

// class condition_variable;

// void wait(unique_lock<mutex>& lock);

#include <condition_variable>
#include <mutex>
#include <thread>
#include <cassert>

std::condition_variable cv;
std::mutex mut;

int test1 = 0;
int test2 = 0;

void f()
{
    std::unique_lock<std::mutex> lk(mut);
    assert(test2 == 0);
    test1 = 1;
    cv.notify_one();
    while (test2 == 0)
        cv.wait(lk);
    assert(test2 != 0);
}

int main()
{
    std::unique_lock<std::mutex>lk(mut);
    std::thread t(f);
    assert(test1 == 0);
    while (test1 == 0)
        cv.wait(lk);
    assert(test1 != 0);
    test2 = 1;
    lk.unlock();
    cv.notify_one();
    t.join();
}
