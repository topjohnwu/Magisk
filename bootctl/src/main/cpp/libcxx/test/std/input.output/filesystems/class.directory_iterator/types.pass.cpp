//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03

// <filesystem>

// class directory_iterator

// typedef ... value_type;
// typedef ... difference_type;
// typedef ... pointer;
// typedef ... reference;
// typedef ... iterator_category

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"


int main() {
    using namespace fs;
    using D = directory_iterator;
    ASSERT_SAME_TYPE(D::value_type, directory_entry);
    ASSERT_SAME_TYPE(D::difference_type, std::ptrdiff_t);
    ASSERT_SAME_TYPE(D::pointer, const directory_entry*);
    ASSERT_SAME_TYPE(D::reference, const directory_entry&);
    ASSERT_SAME_TYPE(D::iterator_category, std::input_iterator_tag);
}
