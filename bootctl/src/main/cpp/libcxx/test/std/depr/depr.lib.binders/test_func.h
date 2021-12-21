//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef TEST_FUNC_H
#define TEST_FUNC_H

class test_func
{
    int id_;
public:
    typedef int first_argument_type;
    typedef double second_argument_type;
    typedef long double result_type;

    explicit test_func(int id) : id_(id) {}

    int id() const {return id_;}

    result_type operator() (const first_argument_type& x, second_argument_type& y) const
        {return x+y;}
    result_type operator() (const first_argument_type& x, const second_argument_type& y) const
        {return x-y;}
    result_type operator() (first_argument_type& x, const second_argument_type& y) const
        {return x*y;}
};

#endif  // TEST_FUNC_H
