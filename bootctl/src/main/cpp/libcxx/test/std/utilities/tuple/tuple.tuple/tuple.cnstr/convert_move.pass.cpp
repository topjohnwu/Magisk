//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <tuple>

// template <class... Types> class tuple;

// template <class... UTypes> tuple(tuple<UTypes...>&& u);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <string>
#include <memory>
#include <cassert>

struct Explicit {
  int value;
  explicit Explicit(int x) : value(x) {}
};

struct Implicit {
  int value;
  Implicit(int x) : value(x) {}
};

struct B
{
    int id_;

    explicit B(int i) : id_(i) {}

    virtual ~B() {}
};

struct D
    : B
{
    explicit D(int i) : B(i) {}
};

int main()
{
    {
        typedef std::tuple<long> T0;
        typedef std::tuple<long long> T1;
        T0 t0(2);
        T1 t1 = std::move(t0);
        assert(std::get<0>(t1) == 2);
    }
    {
        typedef std::tuple<long, char> T0;
        typedef std::tuple<long long, int> T1;
        T0 t0(2, 'a');
        T1 t1 = std::move(t0);
        assert(std::get<0>(t1) == 2);
        assert(std::get<1>(t1) == int('a'));
    }
    {
        typedef std::tuple<long, char, D> T0;
        typedef std::tuple<long long, int, B> T1;
        T0 t0(2, 'a', D(3));
        T1 t1 = std::move(t0);
        assert(std::get<0>(t1) == 2);
        assert(std::get<1>(t1) == int('a'));
        assert(std::get<2>(t1).id_ == 3);
    }
    {
        D d(3);
        typedef std::tuple<long, char, D&> T0;
        typedef std::tuple<long long, int, B&> T1;
        T0 t0(2, 'a', d);
        T1 t1 = std::move(t0);
        d.id_ = 2;
        assert(std::get<0>(t1) == 2);
        assert(std::get<1>(t1) == int('a'));
        assert(std::get<2>(t1).id_ == 2);
    }
    {
        typedef std::tuple<long, char, std::unique_ptr<D>> T0;
        typedef std::tuple<long long, int, std::unique_ptr<B>> T1;
        T0 t0(2, 'a', std::unique_ptr<D>(new D(3)));
        T1 t1 = std::move(t0);
        assert(std::get<0>(t1) == 2);
        assert(std::get<1>(t1) == int('a'));
        assert(std::get<2>(t1)->id_ == 3);
    }
    {
        std::tuple<int> t1(42);
        std::tuple<Explicit> t2(std::move(t1));
        assert(std::get<0>(t2).value == 42);
    }
    {
        std::tuple<int> t1(42);
        std::tuple<Implicit> t2 = std::move(t1);
        assert(std::get<0>(t2).value == 42);
    }
}
