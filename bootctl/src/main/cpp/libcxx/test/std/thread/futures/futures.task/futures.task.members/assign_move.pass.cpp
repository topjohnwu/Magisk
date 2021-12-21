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

// packaged_task& operator=(packaged_task&& other);

#include <future>
#include <cassert>

class A
{
    long data_;

public:
    explicit A(long i) : data_(i) {}

    long operator()(long i, long j) const {return data_ + i + j;}
};

int main()
{
    {
        std::packaged_task<double(int, char)> p0(A(5));
        std::packaged_task<double(int, char)> p;
        p = std::move(p0);
        assert(!p0.valid());
        assert(p.valid());
        std::future<double> f = p.get_future();
        p(3, 'a');
        assert(f.get() == 105.0);
    }
    {
        std::packaged_task<double(int, char)> p0;
        std::packaged_task<double(int, char)> p;
        p = std::move(p0);
        assert(!p0.valid());
        assert(!p.valid());
    }
}
