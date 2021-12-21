//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <memory>

// template <class T>
//   pair<T*, ptrdiff_t>
//   get_temporary_buffer(ptrdiff_t n);
//
// template <class T>
//   void
//   return_temporary_buffer(T* p);

#include <memory>
#include <cassert>

struct alignas(32) A {
    int field;
};

int main()
{
    std::pair<A*, std::ptrdiff_t> ip = std::get_temporary_buffer<A>(5);
    assert(!(ip.first == nullptr) ^ (ip.second == 0));
    assert(reinterpret_cast<uintptr_t>(ip.first) % alignof(A) == 0);
    std::return_temporary_buffer(ip.first);
}
