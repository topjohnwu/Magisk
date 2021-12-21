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
// class binder1st
//   : public unary_function<typename Fn::second_argument_type, typename Fn::result_type>
// {
// protected:
//   Fn op;
//   typename Fn::first_argument_type value;
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
    : public std::binder1st<test_func>
{
    typedef std::binder1st<test_func> base;
public:
    test() : std::binder1st<test_func>(test_func(2), 30) {}

    void do_test()
    {
        static_assert((std::is_base_of<
                         std::unary_function<test_func::second_argument_type,
                                             test_func::result_type>,
                         test>::value), "");
        assert(op.id() == 2);
        assert(value == 30);

        double d = 5;
        assert((*this)(d) == 35);
        assert((*this)(5) == 25);
    }
};

int main()
{
    test t;
    t.do_test();
}
