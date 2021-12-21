//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// type_traits

// extension

// template <typename _Tp> struct __has_operator_addressof


#include <type_traits>


struct A
{
};

struct B
{
    constexpr B* operator&() const;
};

struct D;

struct C
{
    template <class U>
    D operator,(U&&);
};

struct E
{
    constexpr C operator&() const;
};

struct F {};
constexpr F* operator&(F const &) { return nullptr; }

struct G {};
constexpr G* operator&(G &&) { return nullptr; }

struct H {};
constexpr H* operator&(H const &&) { return nullptr; }

struct J
{
    constexpr J* operator&() const &&;
};


int main()
{
    static_assert(std::__has_operator_addressof<int>::value == false, "");
    static_assert(std::__has_operator_addressof<A>::value == false, "");
    static_assert(std::__has_operator_addressof<B>::value == true, "");
    static_assert(std::__has_operator_addressof<E>::value == true, "");
    static_assert(std::__has_operator_addressof<F>::value == true, "");
    static_assert(std::__has_operator_addressof<G>::value == true, "");
    static_assert(std::__has_operator_addressof<H>::value == true, "");
    static_assert(std::__has_operator_addressof<J>::value == true, "");
}
