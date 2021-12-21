//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// reference_wrapper

// reference_wrapper& operator=(const reference_wrapper<T>& x);

#include <functional>
#include <cassert>

class functor1
{
};

template <class T>
void
test(T& t)
{
    std::reference_wrapper<T> r(t);
    T t2 = t;
    std::reference_wrapper<T> r2(t2);
    r2 = r;
    assert(&r2.get() == &t);
}

void f() {}
void g() {}

void
test_function()
{
    std::reference_wrapper<void ()> r(f);
    std::reference_wrapper<void ()> r2(g);
    r2 = r;
    assert(&r2.get() == &f);
}

int main()
{
    void (*fp)() = f;
    test(fp);
    test_function();
    functor1 f1;
    test(f1);
    int i = 0;
    test(i);
    const int j = 0;
    test(j);
}
