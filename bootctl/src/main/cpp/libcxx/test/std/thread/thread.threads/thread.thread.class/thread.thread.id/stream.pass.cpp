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

// <thread>

// class thread::id

// template<class charT, class traits>
// basic_ostream<charT, traits>&
// operator<<(basic_ostream<charT, traits>& out, thread::id id);

#include <thread>
#include <sstream>
#include <cassert>

int main()
{
    std::thread::id id0 = std::this_thread::get_id();
    std::ostringstream os;
    os << id0;
}
