// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// REQUIRES: c++experimental
// UNSUPPORTED: c++98, c++03

// <experimental/regex>

// namespace std { namespace experimental { namespace pmr {
//
//  template <class BidirectionalIterator>
//  using match_results =
//    std::match_results<BidirectionalIterator,
//                       polymorphic_allocator<sub_match<BidirectionalIterator>>>;
//
//  typedef match_results<const char*> cmatch;
//  typedef match_results<const wchar_t*> wcmatch;
//  typedef match_results<string::const_iterator> smatch;
//  typedef match_results<wstring::const_iterator> wsmatch;
//
// }}} // namespace std::experimental::pmr

#include <experimental/regex>
#include <type_traits>
#include <cassert>

namespace pmr = std::experimental::pmr;

template <class Iter, class PmrTypedef>
void test_match_result_typedef() {
    using StdMR = std::match_results<Iter, pmr::polymorphic_allocator<std::sub_match<Iter>>>;
    using PmrMR = pmr::match_results<Iter>;
    static_assert(std::is_same<StdMR, PmrMR>::value, "");
    static_assert(std::is_same<PmrMR, PmrTypedef>::value, "");
}

int main()
{
    {
        test_match_result_typedef<const char*, pmr::cmatch>();
        test_match_result_typedef<const wchar_t*, pmr::wcmatch>();
        test_match_result_typedef<pmr::string::const_iterator, pmr::smatch>();
        test_match_result_typedef<pmr::wstring::const_iterator, pmr::wsmatch>();
    }
    {
        // Check that std::match_results has been included and is complete.
        pmr::smatch s;
        assert(s.get_allocator().resource() == pmr::get_default_resource());
    }
}
