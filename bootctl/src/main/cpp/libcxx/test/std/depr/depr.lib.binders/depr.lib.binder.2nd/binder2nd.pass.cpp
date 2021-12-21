//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <functional>
// REQUIRES: c++98 || c++03 || c++11 || c++14

// template <class Fn>
// class binder2nd
//   : public unary_function<typename Fn::first_argument_type, typename Fn::result_type>
// {
// protected:
//   Fn op;
//   typename Fn::second_argument_type value;
// public:
//   binder2nd(const Fn& x, const typename Fn::second_argument_type& y);
//
//   typename Fn::result_type operator()(const typename Fn::first_argument_type& x) const;
//   typename Fn::result_type operator()(typename Fn::first_argument_type& x) const;
// };

#include <functional>
#include <type_traits>
#include <cassert>

#include "../test_func.h"

class test
    : public std::binder2nd<test_func>
{
    typedef std::binder2nd<test_func> base;
public:
    test() : std::binder2nd<test_func>(test_func(3), 4.5) {}

    void do_test()
    {
        static_assert((std::is_base_of<
                         std::unary_function<test_func::first_argument_type,
                                             test_func::result_type>,
                         test>::value), "");
        assert(op.id() == 3);
        assert(value == 4.5);

        int i = 5;
        assert((*this)(i) == 22.5);
        assert((*this)(5) == 0.5);
    }
};

int main()
{
    test t;
    t.do_test();
}
