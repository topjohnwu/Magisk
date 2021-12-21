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

// uintmax_t hard_link_count(const path& p);
// uintmax_t hard_link_count(const path& p, std::error_code& ec) noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(hard_link_count_test_suite)

TEST_CASE(signature_test)
{
    const path p; ((void)p);
    std::error_code ec; ((void)ec);
    ASSERT_SAME_TYPE(decltype(hard_link_count(p)), uintmax_t);
    ASSERT_SAME_TYPE(decltype(hard_link_count(p, ec)), uintmax_t);
    ASSERT_NOT_NOEXCEPT(hard_link_count(p));
    ASSERT_NOEXCEPT(hard_link_count(p, ec));
}

TEST_CASE(hard_link_count_for_file)
{
    TEST_CHECK(hard_link_count(StaticEnv::File) == 1);
    std::error_code ec;
    TEST_CHECK(hard_link_count(StaticEnv::File, ec) == 1);
}

TEST_CASE(hard_link_count_for_directory)
{
    uintmax_t DirExpect = 3; // hard link from . .. and Dir2
    uintmax_t Dir3Expect = 2; // hard link from . ..
    uintmax_t DirExpectAlt = DirExpect;
    uintmax_t Dir3ExpectAlt = Dir3Expect;
#if defined(__APPLE__)
    // Filesystems formatted with case sensitive hfs+ behave unixish as
    // expected. Normal hfs+ filesystems report the number of directory
    // entries instead.
    DirExpectAlt = 5; // .  ..  Dir2  file1  file2
    Dir3Expect = 3; // .  ..  file5
#endif
    TEST_CHECK(hard_link_count(StaticEnv::Dir) == DirExpect ||
               hard_link_count(StaticEnv::Dir) == DirExpectAlt ||
               hard_link_count(StaticEnv::Dir) == 1);
    TEST_CHECK(hard_link_count(StaticEnv::Dir3) == Dir3Expect ||
               hard_link_count(StaticEnv::Dir3) == Dir3ExpectAlt ||
               hard_link_count(StaticEnv::Dir3) == 1);

    std::error_code ec;
    TEST_CHECK(hard_link_count(StaticEnv::Dir, ec) == DirExpect ||
               hard_link_count(StaticEnv::Dir, ec) == DirExpectAlt ||
               hard_link_count(StaticEnv::Dir) == 1);
    TEST_CHECK(hard_link_count(StaticEnv::Dir3, ec) == Dir3Expect ||
               hard_link_count(StaticEnv::Dir3, ec) == Dir3ExpectAlt ||
               hard_link_count(StaticEnv::Dir3) == 1);
}
TEST_CASE(hard_link_count_increments_test)
{
    scoped_test_env env;
    const path file = env.create_file("file", 42);
    TEST_CHECK(hard_link_count(file) == 1);

    env.create_hardlink(file, "file_hl");
    TEST_CHECK(hard_link_count(file) == 2);
}


TEST_CASE(hard_link_count_error_cases)
{
    const path testCases[] = {
        StaticEnv::BadSymlink,
        StaticEnv::DNE
    };
    const uintmax_t expect = static_cast<uintmax_t>(-1);
    for (auto& TC : testCases) {
        std::error_code ec;
        TEST_CHECK(hard_link_count(TC, ec) == expect);
        TEST_CHECK(ec);
    }
}

TEST_SUITE_END()
