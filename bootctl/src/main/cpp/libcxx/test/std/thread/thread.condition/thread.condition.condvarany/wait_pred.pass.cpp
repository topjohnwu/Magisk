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

// class condition_variable_any;

// template <class Lock, class Predicate>
//   void wait(Lock& lock, Predicate pred);

#include <condition_variable>
#include <mutex>
#include <thread>
#include <functional>
#include <cassert>

std::condition_variable_any cv;

typedef std::timed_mutex L0;
typedef std::unique_lock<L0> L1;

L0 m0;

int test1 = 0;
int test2 = 0;

class Pred
{
    int& i_;
public:
    explicit Pred(int& i) : i_(i) {}

    bool operator()() {return i_ != 0;}
};

void f()
{
    L1 lk(m0);
    assert(test2 == 0);
    test1 = 1;
    cv.notify_one();
    cv.wait(lk, Pred(test2));
    assert(test2 != 0);
}

int main()
{
    L1 lk(m0);
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
