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

// class filesystem_error

// filesystem_error(const string& what_arg, error_code ec);
// filesystem_error(const string& what_arg, const path& p1, error_code ec);
// filesystem_error(const string& what_arg, const path& p1, const path& p2, error_code ec);
// const std::error_code& code() const;
// const char* what() const noexcept;
// const path& path1() const noexcept;
// const path& path2() const noexcept;

#include "filesystem_include.hpp"
#include <type_traits>
#include <cassert>

#include "test_macros.h"


void test_constructors() {
  using namespace fs;

  // The string returned by "filesystem_error::what() must contain runtime_error::what()
  const std::string what_arg = "Hello World";
  const std::string what_contains = std::runtime_error(what_arg).what();
  assert(what_contains.find(what_arg) != std::string::npos);
  auto CheckWhat = [what_contains](filesystem_error const& e) {
    std::string s = e.what();
    assert(s.find(what_contains) != std::string::npos);
  };

  std::error_code ec = std::make_error_code(std::errc::file_exists);
  const path p1("foo");
  const path p2("bar");

  // filesystem_error(const string& what_arg, error_code ec);
  {
    ASSERT_NOT_NOEXCEPT(filesystem_error(what_arg, ec));
    filesystem_error e(what_arg, ec);
    CheckWhat(e);
    assert(e.code() == ec);
    assert(e.path1().empty() && e.path2().empty());
  }
  // filesystem_error(const string& what_arg, const path&, error_code ec);
  {
    ASSERT_NOT_NOEXCEPT(filesystem_error(what_arg, p1, ec));
    filesystem_error e(what_arg, p1, ec);
    CheckWhat(e);
    assert(e.code() == ec);
    assert(e.path1() == p1);
    assert(e.path2().empty());
  }
  // filesystem_error(const string& what_arg, const path&, const path&, error_code ec);
  {
    ASSERT_NOT_NOEXCEPT(filesystem_error(what_arg, p1, p2, ec));
    filesystem_error e(what_arg, p1, p2, ec);
    CheckWhat(e);
    assert(e.code() == ec);
    assert(e.path1() == p1);
    assert(e.path2() == p2);
  }
}

void test_signatures()
{
  using namespace fs;
  const path p;
  std::error_code ec;
  const filesystem_error e("lala", ec);
  // const path& path1() const noexcept;
  {
    ASSERT_SAME_TYPE(path const&, decltype(e.path1()));
    ASSERT_NOEXCEPT(e.path1());
  }
  // const path& path2() const noexcept
  {
    ASSERT_SAME_TYPE(path const&, decltype(e.path2()));
    ASSERT_NOEXCEPT(e.path2());
  }
  // const char* what() const noexcept
  {
    ASSERT_SAME_TYPE(const char*, decltype(e.what()));
    ASSERT_NOEXCEPT(e.what());
  }
}

int main() {
  static_assert(std::is_base_of<std::system_error, fs::filesystem_error>::value, "");
  test_constructors();
  test_signatures();
}
