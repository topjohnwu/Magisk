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

// directory_iterator& operator=(directory_iterator const&);

#include "filesystem_include.hpp"
#include <type_traits>
#include <set>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(directory_iterator_copy_assign_tests)

TEST_CASE(test_assignment_signature)
{
    using D = directory_iterator;
    static_assert(std::is_copy_assignable<D>::value, "");
}

TEST_CASE(test_copy_to_end_iterator)
{
    const path testDir = StaticEnv::Dir;

    const directory_iterator from(testDir);
    TEST_REQUIRE(from != directory_iterator{});
    const path entry = *from;

    directory_iterator to{};
    to = from;
    TEST_REQUIRE(to == from);
    TEST_CHECK(*to == entry);
    TEST_CHECK(*from == entry);
}


TEST_CASE(test_copy_from_end_iterator)
{
    const path testDir = StaticEnv::Dir;

    const directory_iterator from{};

    directory_iterator to(testDir);
    TEST_REQUIRE(to != directory_iterator{});

    to = from;
    TEST_REQUIRE(to == from);
    TEST_CHECK(to == directory_iterator{});
}

TEST_CASE(test_copy_valid_iterator)
{
    const path testDir = StaticEnv::Dir;
    const directory_iterator endIt{};

    directory_iterator it_obj(testDir);
    const directory_iterator& it = it_obj;
    TEST_REQUIRE(it != endIt);
    ++it_obj;
    TEST_REQUIRE(it != endIt);
    const path entry = *it;

    directory_iterator it2(testDir);
    TEST_REQUIRE(it2 != it);
    const path entry2 = *it2;
    TEST_CHECK(entry2 != entry);

    it2 = it;
    TEST_REQUIRE(it2 == it);
    TEST_CHECK(*it2 == entry);
}

TEST_CASE(test_returns_reference_to_self)
{
    const directory_iterator it;
    directory_iterator it2;
    directory_iterator& ref = (it2 = it);
    TEST_CHECK(&ref == &it2);
}


TEST_SUITE_END()
