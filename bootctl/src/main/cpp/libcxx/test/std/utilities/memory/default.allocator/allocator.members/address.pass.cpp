//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <memory>

// allocator:
// pointer address(reference x) const;
// const_pointer address(const_reference x) const;

#include <memory>
#include <cassert>

template <class T>
void test_address()
{
    T* tp = new T();
    const T* ctp = tp;
    const std::allocator<T> a;
    assert(a.address(*tp) == tp);
    assert(a.address(*ctp) == tp);
    delete tp;
}

struct A
{
    void operator&() const {}
};

int main()
{
    test_address<int>();
    test_address<A>();
}
