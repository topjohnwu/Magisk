//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: c++98, c++03

// <tuple>

// template <class... Types> class tuple;

// template <class Alloc> tuple(allocator_arg_t, Alloc const&)

// Libc++ has to deduce the 'allocator_arg_t' parameter for this constructor
// as 'AllocArgT'. Previously libc++ has tried to support tags derived from
// 'allocator_arg_t' by using 'is_base_of<AllocArgT, allocator_arg_t>'.
// However this breaks whenever a 2-tuple contains a reference to an incomplete
// type as its first parameter. See PR27684.

#include <tuple>
#include <cassert>

struct IncompleteType;
extern IncompleteType inc1;
extern IncompleteType inc2;
IncompleteType const& cinc1 = inc1;
IncompleteType const& cinc2 = inc2;

int main() {
    using IT = IncompleteType;
    { // try calling tuple(Tp const&...)
        using Tup = std::tuple<const IT&, const IT&>;
        Tup t(cinc1, cinc2);
        assert(&std::get<0>(t) == &inc1);
        assert(&std::get<1>(t) == &inc2);
    }
    { // try calling tuple(Up&&...)
        using Tup = std::tuple<const IT&, const IT&>;
        Tup t(inc1, inc2);
        assert(&std::get<0>(t) == &inc1);
        assert(&std::get<1>(t) == &inc2);
    }
}

struct IncompleteType {};
IncompleteType inc1;
IncompleteType inc2;
