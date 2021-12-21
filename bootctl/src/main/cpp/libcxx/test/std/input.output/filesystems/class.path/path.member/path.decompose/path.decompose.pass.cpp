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

// class path

// 8.4.9 path decomposition [path.decompose]
//------------------------------------------
// path root_name() const;
// path root_directory() const;
// path root_path() const;
// path relative_path() const;
// path parent_path() const;
// path filename() const;
// path stem() const;
// path extension() const;
//-------------------------------
// 8.4.10 path query [path.query]
//-------------------------------
// bool empty() const noexcept;
// bool has_root_path() const;
// bool has_root_name() const;
// bool has_root_directory() const;
// bool has_relative_path() const;
// bool has_parent_path() const;
// bool has_filename() const;
// bool has_stem() const;
// bool has_extension() const;
// bool is_absolute() const;
// bool is_relative() const;
//-------------------------------
// 8.5 path iterators [path.itr]
//-------------------------------
// iterator begin() const;
// iterator end() const;


#include "filesystem_include.hpp"
#include <type_traits>
#include <vector>
#include <cassert>

#include "test_macros.h"
#include "test_iterators.h"
#include "count_new.hpp"
#include "filesystem_test_helper.hpp"
#include "assert_checkpoint.h"
#include "verbose_assert.h"

struct ComparePathExact {
  bool operator()(std::string const& LHS, std::string const& RHS) const {
    return LHS == RHS;
  }
};

struct PathDecomposeTestcase
{
    std::string raw;
    std::vector<std::string> elements;
    std::string root_path;
    std::string root_name;
    std::string root_directory;
    std::string relative_path;
    std::string parent_path;
    std::string filename;
};

const PathDecomposeTestcase PathTestCases[] =
  {
      {"", {}, "", "", "", "", "", ""}
    , {".", {"."}, "", "", "", ".", "", "."}
    , {"..", {".."}, "", "", "", "..", "", ".."}
    , {"foo", {"foo"}, "", "", "", "foo", "", "foo"}
    , {"/", {"/"}, "/", "", "/", "", "/", ""}
    , {"/foo", {"/", "foo"}, "/", "", "/", "foo", "/", "foo"}
    , {"foo/", {"foo", ""}, "", "", "", "foo/", "foo", ""}
    , {"/foo/", {"/", "foo", ""}, "/", "", "/", "foo/", "/foo", ""}
    , {"foo/bar", {"foo","bar"}, "",  "", "",  "foo/bar", "foo", "bar"}
    , {"/foo//bar", {"/","foo","bar"}, "/", "", "/", "foo/bar", "/foo", "bar"}
    , {"//net", {"/", "net"}, "/", "", "/", "net", "/", "net"}
    , {"//net/foo", {"/", "net", "foo"}, "/", "", "/", "net/foo", "/net", "foo"}
    , {"///foo///", {"/", "foo", ""}, "/", "", "/", "foo///", "///foo", ""}
    , {"///foo///bar", {"/", "foo", "bar"}, "/", "", "/", "foo///bar", "///foo", "bar"}
    , {"/.", {"/", "."}, "/", "", "/", ".", "/", "."}
    , {"./", {".", ""}, "", "", "", "./", ".", ""}
    , {"/..", {"/", ".."}, "/", "", "/", "..", "/", ".."}
    , {"../", {"..", ""}, "", "", "", "../", "..", ""}
    , {"foo/.", {"foo", "."}, "", "", "", "foo/.", "foo", "."}
    , {"foo/..", {"foo", ".."}, "", "", "", "foo/..", "foo", ".."}
    , {"foo/./", {"foo", ".", ""}, "", "", "", "foo/./", "foo/.", ""}
    , {"foo/./bar", {"foo", ".", "bar"}, "", "", "", "foo/./bar", "foo/.", "bar"}
    , {"foo/../", {"foo", "..", ""}, "", "", "", "foo/../", "foo/..", ""}
    , {"foo/../bar", {"foo", "..", "bar"}, "", "", "", "foo/../bar", "foo/..", "bar"}
    , {"c:", {"c:"}, "", "", "", "c:", "", "c:"}
    , {"c:/", {"c:", ""}, "", "", "", "c:/", "c:", ""}
    , {"c:foo", {"c:foo"}, "", "", "", "c:foo", "", "c:foo"}
    , {"c:/foo", {"c:", "foo"}, "", "", "", "c:/foo", "c:", "foo"}
    , {"c:foo/", {"c:foo", ""}, "", "", "", "c:foo/", "c:foo", ""}
    , {"c:/foo/", {"c:", "foo", ""}, "", "", "", "c:/foo/",  "c:/foo", ""}
    , {"c:/foo/bar", {"c:", "foo", "bar"}, "", "", "", "c:/foo/bar", "c:/foo", "bar"}
    , {"prn:", {"prn:"}, "", "", "", "prn:", "", "prn:"}
    , {"c:\\", {"c:\\"}, "", "", "", "c:\\", "", "c:\\"}
    , {"c:\\foo", {"c:\\foo"}, "", "", "", "c:\\foo", "", "c:\\foo"}
    , {"c:foo\\", {"c:foo\\"}, "", "", "", "c:foo\\", "", "c:foo\\"}
    , {"c:\\foo\\", {"c:\\foo\\"}, "", "", "", "c:\\foo\\", "", "c:\\foo\\"}
    , {"c:\\foo/",  {"c:\\foo", ""}, "", "", "", "c:\\foo/", "c:\\foo", ""}
    , {"c:/foo\\bar", {"c:", "foo\\bar"}, "", "", "", "c:/foo\\bar", "c:", "foo\\bar"}
    , {"//", {"/"}, "/", "", "/", "", "/", ""}
  };

void decompPathTest()
{
  using namespace fs;
  for (auto const & TC : PathTestCases) {
    CHECKPOINT(TC.raw.c_str());
    fs::path p(TC.raw);
    ASSERT(p == TC.raw);

    ASSERT_EQ(p.root_path(), TC.root_path);
    ASSERT_NEQ(p.has_root_path(), TC.root_path.empty());

    ASSERT(p.root_name().native().empty())
        << DISPLAY(p.root_name());
    ASSERT_EQ(p.root_name(),TC.root_name);
    ASSERT_NEQ(p.has_root_name(), TC.root_name.empty());

    ASSERT_EQ(p.root_directory(), TC.root_directory);
    ASSERT_NEQ(p.has_root_directory(), TC.root_directory.empty());

    ASSERT_EQ(p.relative_path(), TC.relative_path);
    ASSERT_NEQ(p.has_relative_path(), TC.relative_path.empty());

    ASSERT_EQ(p.parent_path(), TC.parent_path);
    ASSERT_NEQ(p.has_parent_path(), TC.parent_path.empty());

    ASSERT_EQ(p.filename(), TC.filename);
    ASSERT_NEQ(p.has_filename(), TC.filename.empty());

    ASSERT_EQ(p.is_absolute(), p.has_root_directory());
    ASSERT_NEQ(p.is_relative(), p.is_absolute());
    if (p.empty())
      ASSERT(p.is_relative());

    ASSERT_COLLECTION_EQ_COMP(
        p.begin(), p.end(),
        TC.elements.begin(), TC.elements.end(),
        ComparePathExact()
    );
    // check backwards

    std::vector<fs::path> Parts;
    for (auto it = p.end(); it != p.begin(); )
      Parts.push_back(*--it);
    ASSERT_COLLECTION_EQ_COMP(Parts.begin(), Parts.end(),
                                 TC.elements.rbegin(), TC.elements.rend(),
                              ComparePathExact());
  }
}


struct FilenameDecompTestcase
{
  std::string raw;
  std::string filename;
  std::string stem;
  std::string extension;
};

const FilenameDecompTestcase FilenameTestCases[] =
{
    {"", "", "", ""}
  , {".", ".", ".", ""}
  , {"..", "..", "..", ""}
  , {"/", "", "", ""}
  , {"foo", "foo", "foo", ""}
  , {"/foo/bar.txt", "bar.txt", "bar", ".txt"}
  , {"foo..txt", "foo..txt", "foo.", ".txt"}
  , {".profile", ".profile", ".profile", ""}
  , {".profile.txt", ".profile.txt", ".profile", ".txt"}
};


void decompFilenameTest()
{
  using namespace fs;
  for (auto const & TC : FilenameTestCases) {
    CHECKPOINT(TC.raw.c_str());
    fs::path p(TC.raw);
    ASSERT_EQ(p, TC.raw);
    ASSERT_NOEXCEPT(p.empty());

    ASSERT_EQ(p.filename(), TC.filename);
    ASSERT_NEQ(p.has_filename(), TC.filename.empty());

    ASSERT_EQ(p.stem(), TC.stem);
    ASSERT_NEQ(p.has_stem(), TC.stem.empty());

    ASSERT_EQ(p.extension(), TC.extension);
    ASSERT_NEQ(p.has_extension(), TC.extension.empty());
  }
}

int main()
{
  decompPathTest();
  decompFilenameTest();
}
