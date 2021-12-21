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

// path canonical(const path& p);
// path canonical(const path& p, error_code& ec);

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"
#include "rapid-cxx-test.hpp"
#include "filesystem_test_helper.hpp"

using namespace fs;

struct CWDGuard {
  path OldCWD;
  CWDGuard() : OldCWD(fs::current_path()) { }
  ~CWDGuard() { fs::current_path(OldCWD); }

  CWDGuard(CWDGuard const&) = delete;
  CWDGuard& operator=(CWDGuard const&) = delete;
};

TEST_SUITE(filesystem_canonical_path_test_suite)

TEST_CASE(signature_test)
{
    const path p; ((void)p);
    std::error_code ec; ((void)ec);
    ASSERT_NOT_NOEXCEPT(canonical(p));
    ASSERT_NOT_NOEXCEPT(canonical(p, ec));
}

// There are 4 cases is the proposal for absolute path.
// Each scope tests one of the cases.
TEST_CASE(test_canonical)
{
    CWDGuard guard;
    // has_root_name() && has_root_directory()
    const path Root = StaticEnv::Root;
    const path RootName = Root.filename();
    const path DirName = StaticEnv::Dir.filename();
    const path SymlinkName = StaticEnv::SymlinkToFile.filename();
    struct TestCase {
        path p;
        path expect;
        path base;
        TestCase(path p1, path e, path b = StaticEnv::Root)
            : p(p1), expect(e), base(b) {}
    };
    const TestCase testCases[] = {
        { ".", Root, Root},
        { DirName / ".." / "." / DirName, StaticEnv::Dir, Root},
        { StaticEnv::Dir2 / "..",    StaticEnv::Dir },
        { StaticEnv::Dir3 / "../..", StaticEnv::Dir },
        { StaticEnv::Dir / ".",      StaticEnv::Dir },
        { Root / "." / DirName / ".." / DirName, StaticEnv::Dir},
        { path("..") / "." / RootName / DirName / ".." / DirName, StaticEnv::Dir, Root},
        { StaticEnv::SymlinkToFile,  StaticEnv::File },
        { SymlinkName, StaticEnv::File, StaticEnv::Root}
    };
    for (auto& TC : testCases) {
        std::error_code ec = GetTestEC();
        fs::current_path(TC.base);
        const path ret = canonical(TC.p, ec);
        TEST_REQUIRE(!ec);
        const path ret2 = canonical(TC.p);
        TEST_CHECK(PathEq(ret, TC.expect));
        TEST_CHECK(PathEq(ret, ret2));
        TEST_CHECK(ret.is_absolute());
    }
}

TEST_CASE(test_dne_path)
{
    std::error_code ec = GetTestEC();
    {
        const path ret = canonical(StaticEnv::DNE, ec);
        TEST_CHECK(ec != GetTestEC());
        TEST_REQUIRE(ec);
        TEST_CHECK(ret == path{});
    }
    {
        TEST_CHECK_THROW(filesystem_error, canonical(StaticEnv::DNE));
    }
}

TEST_CASE(test_exception_contains_paths)
{
#ifndef TEST_HAS_NO_EXCEPTIONS
    CWDGuard guard;
    const path p = "blabla/dne";
    try {
        canonical(p);
        TEST_REQUIRE(false);
    } catch (filesystem_error const& err) {
        TEST_CHECK(err.path1() == p);
        // libc++ provides the current path as the second path in the exception
        LIBCPP_ONLY(TEST_CHECK(err.path2() == current_path()));
    }
    fs::current_path(StaticEnv::Dir);
    try {
        canonical(p);
        TEST_REQUIRE(false);
    } catch (filesystem_error const& err) {
        TEST_CHECK(err.path1() == p);
        LIBCPP_ONLY(TEST_CHECK(err.path2() == StaticEnv::Dir));
    }
#endif
}

TEST_SUITE_END()
