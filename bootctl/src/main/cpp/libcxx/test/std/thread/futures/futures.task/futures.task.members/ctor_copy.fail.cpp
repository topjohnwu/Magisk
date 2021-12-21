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
// UNSUPPORTED: c++98, c++03

// <future>

// class packaged_task<R(ArgTypes...)>

// packaged_task(packaged_task&) = delete;

#include <future>


int main()
{
    {
        std::packaged_task<double(int, char)> p0;
        std::packaged_task<double(int, char)> p(p0); // expected-error {{call to deleted constructor of 'std::packaged_task<double (int, char)>'}}
    }
}
