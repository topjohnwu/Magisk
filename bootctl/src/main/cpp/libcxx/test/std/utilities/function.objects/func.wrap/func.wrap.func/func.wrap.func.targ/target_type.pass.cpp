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

// const std::type_info& target_type() const;

#include <functional>
#include <typeinfo>
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
    assert(f.target_type() == typeid(A));
    }
    {
    std::function<int(int)> f;
    assert(f.target_type() == typeid(void));
    }
}
