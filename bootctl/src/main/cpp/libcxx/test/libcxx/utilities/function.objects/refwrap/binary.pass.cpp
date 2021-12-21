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

// check for deriving from binary_function

#include <functional>
#include <type_traits>

class functor1
    : public std::unary_function<int, char>
{
};

class functor2
    : public std::binary_function<char, int, double>
{
};

class functor3
    : public std::unary_function<int, int>,
      public std::binary_function<char, int, double>
{
public:
    typedef float result_type;
};

class functor4
    : public std::unary_function<int, int>,
      public std::binary_function<char, int, double>
{
public:
};

struct C
{
    typedef int argument_type;
    typedef int result_type;
};

int main()
{
    static_assert((!std::is_base_of<std::binary_function<int, char, int>,
                                    std::reference_wrapper<functor1> >::value), "");
    static_assert((std::is_base_of<std::binary_function<char, int, double>,
                                   std::reference_wrapper<functor2> >::value), "");
    static_assert((std::is_base_of<std::binary_function<char, int, double>,
                                   std::reference_wrapper<functor3> >::value), "");
    static_assert((std::is_base_of<std::binary_function<char, int, double>,
                                   std::reference_wrapper<functor4> >::value), "");
    static_assert((!std::is_base_of<std::binary_function<int, int, int>,
                                    std::reference_wrapper<C> >::value), "");
    static_assert((!std::is_base_of<std::binary_function<int, int, float>,
                                    std::reference_wrapper<float ()> >::value), "");
    static_assert((!std::is_base_of<std::binary_function<int, int, float>,
                                   std::reference_wrapper<float (int)> >::value), "");
    static_assert((std::is_base_of<std::binary_function<int, int, float>,
                                    std::reference_wrapper<float (int, int)> >::value), "");
    static_assert((!std::is_base_of<std::binary_function<int, int, float>,
                                    std::reference_wrapper<float(*)()> >::value), "");
    static_assert((!std::is_base_of<std::binary_function<int, int, float>,
                                   std::reference_wrapper<float(*)(int)> >::value), "");
    static_assert((std::is_base_of<std::binary_function<int, int, float>,
                                    std::reference_wrapper<float(*)(int, int)> >::value), "");
    static_assert((!std::is_base_of<std::binary_function<C*, int, float>,
                                   std::reference_wrapper<float(C::*)()> >::value), "");
    static_assert((std::is_base_of<std::binary_function<C*, int, float>,
                                   std::reference_wrapper<float(C::*)(int)> >::value), "");
    static_assert((std::is_base_of<std::binary_function<const volatile C*, int, float>,
                                   std::reference_wrapper<float(C::*)(int) const volatile> >::value), "");
}
