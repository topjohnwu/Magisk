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

// ~condition_variable_any();

#include <condition_variable>
#include <mutex>
#include <thread>
#include <cassert>

std::condition_variable_any* cv;
std::mutex m;

bool f_ready = false;
bool g_ready = false;

void f()
{
    m.lock();
    f_ready = true;
    cv->notify_one();
    delete cv;
    m.unlock();
}

void g()
{
    m.lock();
    g_ready = true;
    cv->notify_one();
    while (!f_ready)
        cv->wait(m);
    m.unlock();
}

int main()
{
    cv = new std::condition_variable_any;
    std::thread th2(g);
    m.lock();
    while (!g_ready)
        cv->wait(m);
    m.unlock();
    std::thread th1(f);
    th1.join();
    th2.join();
}
