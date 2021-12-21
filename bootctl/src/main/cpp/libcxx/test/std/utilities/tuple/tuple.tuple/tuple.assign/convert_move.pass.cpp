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

// template <class... UTypes>
//   tuple& operator=(tuple<UTypes...>&& u);

// UNSUPPORTED: c++98, c++03

#include <tuple>
#include <string>
#include <memory>
#include <utility>
#include <cassert>

struct B
{
    int id_;

    explicit B(int i= 0) : id_(i) {}

    virtual ~B() {}
};

struct D
    : B
{
    explicit D(int i) : B(i) {}
};

struct E {
  E() = default;
  E& operator=(int) {
      return *this;
  }
};

int main()
{
    {
        typedef std::tuple<long> T0;
        typedef std::tuple<long long> T1;
        T0 t0(2);
        T1 t1;
        t1 = std::move(t0);
        assert(std::get<0>(t1) == 2);
    }
    {
        typedef std::tuple<long, char> T0;
        typedef std::tuple<long long, int> T1;
        T0 t0(2, 'a');
        T1 t1;
        t1 = std::move(t0);
        assert(std::get<0>(t1) == 2);
        assert(std::get<1>(t1) == int('a'));
    }
    {
        typedef std::tuple<long, char, D> T0;
        typedef std::tuple<long long, int, B> T1;
        T0 t0(2, 'a', D(3));
        T1 t1;
        t1 = std::move(t0);
        assert(std::get<0>(t1) == 2);
        assert(std::get<1>(t1) == int('a'));
        assert(std::get<2>(t1).id_ == 3);
    }
    {
        D d(3);
        D d2(2);
        typedef std::tuple<long, char, D&> T0;
        typedef std::tuple<long long, int, B&> T1;
        T0 t0(2, 'a', d2);
        T1 t1(1, 'b', d);
        t1 = std::move(t0);
        assert(std::get<0>(t1) == 2);
        assert(std::get<1>(t1) == int('a'));
        assert(std::get<2>(t1).id_ == 2);
    }
    {
        typedef std::tuple<long, char, std::unique_ptr<D>> T0;
        typedef std::tuple<long long, int, std::unique_ptr<B>> T1;
        T0 t0(2, 'a', std::unique_ptr<D>(new D(3)));
        T1 t1;
        t1 = std::move(t0);
        assert(std::get<0>(t1) == 2);
        assert(std::get<1>(t1) == int('a'));
        assert(std::get<2>(t1)->id_ == 3);
    }
    {
        // Test that tuple evaluates correctly applies an lvalue reference
        // before evaluating is_assignable (ie 'is_assignable<int&, int&&>')
        // instead of evaluating 'is_assignable<int&&, int&&>' which is false.
        int x = 42;
        int y = 43;
        std::tuple<int&&, E> t(std::move(x), E{});
        std::tuple<int&&, int> t2(std::move(y), 44);
        t = std::move(t2);
        assert(std::get<0>(t) == 43);
        assert(&std::get<0>(t) == &x);
    }
}
