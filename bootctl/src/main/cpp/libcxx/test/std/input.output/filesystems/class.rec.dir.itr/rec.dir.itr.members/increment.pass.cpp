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

// class recursive_directory_iterator

// recursive_directory_iterator& operator++();
// recursive_directory_iterator& increment(error_code& ec) noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <set>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

TEST_SUITE(recursive_directory_iterator_increment_tests)

TEST_CASE(test_increment_signatures)
{
    recursive_directory_iterator d; ((void)d);
    std::error_code ec; ((void)ec);

    ASSERT_SAME_TYPE(decltype(++d), recursive_directory_iterator&);
    ASSERT_NOT_NOEXCEPT(++d);

    ASSERT_SAME_TYPE(decltype(d.increment(ec)), recursive_directory_iterator&);
    ASSERT_NOT_NOEXCEPT(d.increment(ec));
}

TEST_CASE(test_prefix_increment)
{
    const path testDir = StaticEnv::Dir;
    const std::set<path> dir_contents(std::begin(StaticEnv::RecDirIterationList),
                                      std::end(  StaticEnv::RecDirIterationList));
    const recursive_directory_iterator endIt{};

    std::error_code ec;
    recursive_directory_iterator it(testDir, ec);
    TEST_REQUIRE(!ec);

    std::set<path> unseen_entries = dir_contents;
    while (!unseen_entries.empty()) {
        TEST_REQUIRE(it != endIt);
        const path entry = *it;
        TEST_REQUIRE(unseen_entries.erase(entry) == 1);
        recursive_directory_iterator& it_ref = ++it;
        TEST_CHECK(&it_ref == &it);
    }

    TEST_CHECK(it == endIt);
}

TEST_CASE(test_postfix_increment)
{
    const path testDir = StaticEnv::Dir;
    const std::set<path> dir_contents(std::begin(StaticEnv::RecDirIterationList),
                                      std::end(  StaticEnv::RecDirIterationList));
    const recursive_directory_iterator endIt{};

    std::error_code ec;
    recursive_directory_iterator it(testDir, ec);
    TEST_REQUIRE(!ec);

    std::set<path> unseen_entries = dir_contents;
    while (!unseen_entries.empty()) {
        TEST_REQUIRE(it != endIt);
        const path entry = *it;
        TEST_REQUIRE(unseen_entries.erase(entry) == 1);
        const path entry2 = *it++;
        TEST_CHECK(entry2 == entry);
    }
    TEST_CHECK(it == endIt);
}


TEST_CASE(test_increment_method)
{
    const path testDir = StaticEnv::Dir;
    const std::set<path> dir_contents(std::begin(StaticEnv::RecDirIterationList),
                                      std::end(  StaticEnv::RecDirIterationList));
    const recursive_directory_iterator endIt{};

    std::error_code ec;
    recursive_directory_iterator it(testDir, ec);
    TEST_REQUIRE(!ec);

    std::set<path> unseen_entries = dir_contents;
    while (!unseen_entries.empty()) {
        TEST_REQUIRE(it != endIt);
        const path entry = *it;
        TEST_REQUIRE(unseen_entries.erase(entry) == 1);
        recursive_directory_iterator& it_ref = it.increment(ec);
        TEST_REQUIRE(!ec);
        TEST_CHECK(&it_ref == &it);
    }

    TEST_CHECK(it == endIt);
}

TEST_CASE(test_follow_symlinks)
{
    const path testDir = StaticEnv::Dir;
    auto const& IterList = StaticEnv::RecDirFollowSymlinksIterationList;

    const std::set<path> dir_contents(std::begin(IterList), std::end(IterList));
    const recursive_directory_iterator endIt{};

    std::error_code ec;
    recursive_directory_iterator it(testDir,
                              directory_options::follow_directory_symlink, ec);
    TEST_REQUIRE(!ec);

    std::set<path> unseen_entries = dir_contents;
    while (!unseen_entries.empty()) {
        TEST_REQUIRE(it != endIt);
        const path entry = *it;

        TEST_REQUIRE(unseen_entries.erase(entry) == 1);
        recursive_directory_iterator& it_ref = it.increment(ec);
        TEST_REQUIRE(!ec);
        TEST_CHECK(&it_ref == &it);
    }
    TEST_CHECK(it == endIt);
}

TEST_CASE(access_denied_on_recursion_test_case)
{
    using namespace fs;
    scoped_test_env env;
    const path testFiles[] = {
        env.create_dir("dir1"),
        env.create_dir("dir1/dir2"),
        env.create_file("dir1/dir2/file1"),
        env.create_file("dir1/file2")
    };
    const path startDir = testFiles[0];
    const path permDeniedDir = testFiles[1];
    const path otherFile = testFiles[3];
    auto SkipEPerm = directory_options::skip_permission_denied;

    // Change the permissions so we can no longer iterate
    permissions(permDeniedDir, perms::none);

    const recursive_directory_iterator endIt;

    // Test that recursion resulting in a "EACCESS" error is not ignored
    // by default.
    {
        std::error_code ec = GetTestEC();
        recursive_directory_iterator it(startDir, ec);
        TEST_REQUIRE(ec != GetTestEC());
        TEST_REQUIRE(!ec);
        while (it != endIt && it->path() != permDeniedDir)
            ++it;
        TEST_REQUIRE(it != endIt);
        TEST_REQUIRE(*it == permDeniedDir);

        it.increment(ec);
        TEST_CHECK(ec);
        TEST_CHECK(it == endIt);
    }
    // Same as above but test operator++().
    {
        std::error_code ec = GetTestEC();
        recursive_directory_iterator it(startDir, ec);
        TEST_REQUIRE(!ec);
        while (it != endIt && it->path() != permDeniedDir)
            ++it;
        TEST_REQUIRE(it != endIt);
        TEST_REQUIRE(*it == permDeniedDir);

        TEST_REQUIRE_THROW(filesystem_error, ++it);
    }
    // Test that recursion resulting in a "EACCESS" error is ignored when the
    // correct options are given to the constructor.
    {
        std::error_code ec = GetTestEC();
        recursive_directory_iterator it(startDir, SkipEPerm, ec);
        TEST_REQUIRE(!ec);
        TEST_REQUIRE(it != endIt);

        bool seenOtherFile = false;
        if (*it == otherFile) {
            ++it;
            seenOtherFile = true;
            TEST_REQUIRE (it != endIt);
        }
        TEST_REQUIRE(*it == permDeniedDir);

        ec = GetTestEC();
        it.increment(ec);
        TEST_REQUIRE(!ec);

        if (seenOtherFile) {
            TEST_CHECK(it == endIt);
        } else {
            TEST_CHECK(it != endIt);
            TEST_CHECK(*it == otherFile);
        }
    }
    // Test that construction resulting in a "EACCESS" error is not ignored
    // by default.
    {
        std::error_code ec;
        recursive_directory_iterator it(permDeniedDir, ec);
        TEST_REQUIRE(ec);
        TEST_REQUIRE(it == endIt);
    }
    // Same as above but testing the throwing constructors
    {
        TEST_REQUIRE_THROW(filesystem_error,
                           recursive_directory_iterator(permDeniedDir));
    }
    // Test that construction resulting in a "EACCESS" error constructs the
    // end iterator when the correct options are given.
    {
        std::error_code ec = GetTestEC();
        recursive_directory_iterator it(permDeniedDir, SkipEPerm, ec);
        TEST_REQUIRE(!ec);
        TEST_REQUIRE(it == endIt);
    }
}

// See llvm.org/PR35078
TEST_CASE(test_PR35078)
{
  using namespace fs;
    scoped_test_env env;
    const path testFiles[] = {
        env.create_dir("dir1"),
        env.create_dir("dir1/dir2"),
        env.create_dir("dir1/dir2/dir3"),
        env.create_file("dir1/file1"),
        env.create_file("dir1/dir2/dir3/file2")
    };
    const path startDir = testFiles[0];
    const path permDeniedDir = testFiles[1];
    const path nestedDir = testFiles[2];
    const path nestedFile = testFiles[3];

    // Change the permissions so we can no longer iterate
    permissions(permDeniedDir,
                perms::group_exec|perms::owner_exec|perms::others_exec,
                perm_options::remove);

    const std::error_code eacess_ec =
        std::make_error_code(std::errc::permission_denied);
    std::error_code ec = GetTestEC();

    const recursive_directory_iterator endIt;

    auto SetupState = [&](bool AllowEAccess, bool& SeenFile3) {
      SeenFile3 = false;
      auto Opts = AllowEAccess ? directory_options::skip_permission_denied
          : directory_options::none;
      recursive_directory_iterator it(startDir, Opts, ec);
      while (!ec && it != endIt && *it != nestedDir) {
        if (*it == nestedFile)
          SeenFile3 = true;
        it.increment(ec);
      }
      return it;
    };

    {
      bool SeenNestedFile = false;
      recursive_directory_iterator it = SetupState(false, SeenNestedFile);
      TEST_REQUIRE(it != endIt);
      TEST_REQUIRE(*it == nestedDir);
      ec = GetTestEC();
      it.increment(ec);
      TEST_CHECK(ec);
      TEST_CHECK(ec == eacess_ec);
      TEST_CHECK(it == endIt);
    }
    {
      bool SeenNestedFile = false;
      recursive_directory_iterator it = SetupState(true, SeenNestedFile);
      TEST_REQUIRE(it != endIt);
      TEST_REQUIRE(*it == nestedDir);
      ec = GetTestEC();
      it.increment(ec);
      TEST_CHECK(!ec);
      if (SeenNestedFile) {
        TEST_CHECK(it == endIt);
      } else {
        TEST_REQUIRE(it != endIt);
        TEST_CHECK(*it == nestedFile);
      }
    }
    {
      bool SeenNestedFile = false;
      recursive_directory_iterator it = SetupState(false, SeenNestedFile);
      TEST_REQUIRE(it != endIt);
      TEST_REQUIRE(*it == nestedDir);

      ExceptionChecker Checker(std::errc::permission_denied,
                               "recursive_directory_iterator::operator++()",
                               format_string("attempting recursion into \"%s\"",
                                             nestedDir.native()));
      TEST_CHECK_THROW_RESULT(filesystem_error, Checker, ++it);
    }
}


// See llvm.org/PR35078
TEST_CASE(test_PR35078_with_symlink)
{
  using namespace fs;
    scoped_test_env env;
    const path testFiles[] = {
        env.create_dir("dir1"),
        env.create_file("dir1/file1"),
        env.create_dir("sym_dir"),
        env.create_dir("sym_dir/nested_sym_dir"),
        env.create_symlink("sym_dir/nested_sym_dir", "dir1/dir2"),
        env.create_dir("sym_dir/dir1"),
        env.create_dir("sym_dir/dir1/dir2"),

    };
   // const unsigned TestFilesSize = sizeof(testFiles) / sizeof(testFiles[0]);
    const path startDir = testFiles[0];
    const path nestedFile = testFiles[1];
    const path permDeniedDir = testFiles[2];
    const path symDir = testFiles[4];

    // Change the permissions so we can no longer iterate
    permissions(permDeniedDir,
                perms::group_exec|perms::owner_exec|perms::others_exec,
                perm_options::remove);

    const std::error_code eacess_ec =
        std::make_error_code(std::errc::permission_denied);
    std::error_code ec = GetTestEC();

    const recursive_directory_iterator endIt;

    auto SetupState = [&](bool AllowEAccess, bool FollowSym, bool& SeenFile3) {
      SeenFile3 = false;
      auto Opts = AllowEAccess ? directory_options::skip_permission_denied
          : directory_options::none;
      if (FollowSym)
        Opts |= directory_options::follow_directory_symlink;
      recursive_directory_iterator it(startDir, Opts, ec);
      while (!ec && it != endIt && *it != symDir) {
        if (*it == nestedFile)
          SeenFile3 = true;
        it.increment(ec);
      }
      return it;
    };

    struct {
      bool SkipPermDenied;
      bool FollowSymlinks;
      bool ExpectSuccess;
    } TestCases[]  = {
        // Passing cases
        {false, false, true}, {true, true, true}, {true, false, true},
        // Failing cases
        {false, true, false}
    };
    for (auto TC : TestCases) {
      bool SeenNestedFile = false;
      recursive_directory_iterator it = SetupState(TC.SkipPermDenied,
                                                   TC.FollowSymlinks,
                                                   SeenNestedFile);
      TEST_REQUIRE(!ec);
      TEST_REQUIRE(it != endIt);
      TEST_REQUIRE(*it == symDir);
      ec = GetTestEC();
      it.increment(ec);
      if (TC.ExpectSuccess) {
        TEST_CHECK(!ec);
        if (SeenNestedFile) {
          TEST_CHECK(it == endIt);
        } else {
          TEST_REQUIRE(it != endIt);
          TEST_CHECK(*it == nestedFile);
        }
      } else {
        TEST_CHECK(ec);
        TEST_CHECK(ec == eacess_ec);
        TEST_CHECK(it == endIt);
      }
    }
}


// See llvm.org/PR35078
TEST_CASE(test_PR35078_with_symlink_file)
{
  using namespace fs;
    scoped_test_env env;
    const path testFiles[] = {
        env.create_dir("dir1"),
        env.create_dir("dir1/dir2"),
        env.create_file("dir1/file2"),
        env.create_dir("sym_dir"),
        env.create_dir("sym_dir/sdir1"),
        env.create_file("sym_dir/sdir1/sfile1"),
        env.create_symlink("sym_dir/sdir1/sfile1", "dir1/dir2/file1")
    };
    const unsigned TestFilesSize = sizeof(testFiles) / sizeof(testFiles[0]);
    const path startDir = testFiles[0];
    const path nestedDir = testFiles[1];
    const path nestedFile = testFiles[2];
    const path permDeniedDir = testFiles[3];
    const path symFile = testFiles[TestFilesSize - 1];

    // Change the permissions so we can no longer iterate
    permissions(permDeniedDir,
                perms::group_exec|perms::owner_exec|perms::others_exec,
                perm_options::remove);

    const std::error_code eacess_ec =
        std::make_error_code(std::errc::permission_denied);
    std::error_code ec = GetTestEC();

    const recursive_directory_iterator EndIt;

    auto SetupState = [&](bool AllowEAccess, bool FollowSym, bool& SeenNestedFile) {
      SeenNestedFile = false;
      auto Opts = AllowEAccess ? directory_options::skip_permission_denied
          : directory_options::none;
      if (FollowSym)
        Opts |= directory_options::follow_directory_symlink;
      recursive_directory_iterator it(startDir, Opts, ec);
      while (!ec && it != EndIt && *it != nestedDir) {
        if (*it == nestedFile)
          SeenNestedFile = true;
        it.increment(ec);
      }
      return it;
    };

    struct {
      bool SkipPermDenied;
      bool FollowSymlinks;
      bool ExpectSuccess;
    } TestCases[]  = {
        // Passing cases
        {false, false, true}, {true, true, true}, {true, false, true},
        // Failing cases
        {false, true, false}
    };
    for (auto TC : TestCases){
      bool SeenNestedFile = false;
      recursive_directory_iterator it = SetupState(TC.SkipPermDenied,
                                                   TC.FollowSymlinks,
                                                   SeenNestedFile);
      TEST_REQUIRE(!ec);
      TEST_REQUIRE(it != EndIt);
      TEST_REQUIRE(*it == nestedDir);
      ec = GetTestEC();
      it.increment(ec);
      TEST_REQUIRE(it != EndIt);
      TEST_CHECK(!ec);
      TEST_CHECK(*it == symFile);
      ec = GetTestEC();
      it.increment(ec);
      if (TC.ExpectSuccess) {
        if (!SeenNestedFile) {
          TEST_CHECK(!ec);
          TEST_REQUIRE(it != EndIt);
          TEST_CHECK(*it == nestedFile);
          ec = GetTestEC();
          it.increment(ec);
        }
        TEST_CHECK(!ec);
        TEST_CHECK(it == EndIt);
      } else {
        TEST_CHECK(ec);
        TEST_CHECK(ec == eacess_ec);
        TEST_CHECK(it == EndIt);
      }
    }
}


TEST_SUITE_END()
