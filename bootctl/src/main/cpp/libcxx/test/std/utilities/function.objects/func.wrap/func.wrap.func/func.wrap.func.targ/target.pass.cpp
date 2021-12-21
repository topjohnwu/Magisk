//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>

// class function<R(ArgTypes...)>

// template<typename T>
//   requires Callable<T, ArgTypes...> && Convertible<Callable<T, ArgTypes...>::result_type, R>
//   T*
//   target();
// template<typename T>
//   requires Callable<T, ArgTypes...> && Convertible<Callable<T, ArgTypes...>::result_type, R>
//   const T*
//   target() const;

#include <functional>
#include <new>
#include <cstdlib>
#include <cassert>

class A
{
    int data_[10];
public:
    static int count;

    A()
    {
        ++count;
        for (int i = 0; i < 10; ++i)
            data_[i] = i;
    }

    A(const A&) {++count;}

    ~A() {--count;}

    int operator()(int i) const
    {
        for (int j = 0; j < 10; ++j)
            i += data_[j];
        return i;
    }

    int foo(int) const {return 1;}
};

int A::count = 0;

int g(int) {return 0;}

int main()
{
    {
    std::function<int(int)> f = A();
    assert(A::count == 1);
    assert(f.target<A>());
    assert(f.target<int(*)(int)>() == 0);
    assert(f.target<int>() == nullptr);
    }
    assert(A::count == 0);
    {
    std::function<int(int)> f = g;
    assert(A::count == 0);
    assert(f.target<int(*)(int)>());
    assert(f.target<A>() == 0);
    assert(f.target<int>() == nullptr);
    }
    assert(A::count == 0);
    {
    const std::function<int(int)> f = A();
    assert(A::count == 1);
    assert(f.target<A>());
    assert(f.target<int(*)(int)>() == 0);
    assert(f.target<int>() == nullptr);
    }
    assert(A::count == 0);
    {
    const std::function<int(int)> f = g;
    assert(A::count == 0);
    assert(f.target<int(*)(int)>());
    assert(f.target<A>() == 0);
    assert(f.target<int>() == nullptr);
    }
    assert(A::count == 0);
}
