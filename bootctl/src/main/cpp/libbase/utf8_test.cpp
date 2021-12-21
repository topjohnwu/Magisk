/*
* Copyright (C) 2015 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "android-base/utf8.h"

#include <gtest/gtest.h>

#include <fcntl.h>
#include <stdlib.h>

#include "android-base/file.h"
#include "android-base/macros.h"
#include "android-base/unique_fd.h"

namespace android {
namespace base {

TEST(UTFStringConversionsTest, ConvertInvalidUTF8) {
  std::wstring wide;

  errno = 0;

  // Standalone \xa2 is an invalid UTF-8 sequence, so this should return an
  // error. Concatenate two C/C++ literal string constants to prevent the
  // compiler from giving an error about "\xa2af" containing a "hex escape
  // sequence out of range".
  EXPECT_FALSE(android::base::UTF8ToWide("before\xa2" "after", &wide));

  EXPECT_EQ(EILSEQ, errno);

  // Even if an invalid character is encountered, UTF8ToWide() should still do
  // its best to convert the rest of the string. sysdeps_win32.cpp:
  // _console_write_utf8() depends on this behavior.
  //
  // Thus, we verify that the valid characters are converted, but we ignore the
  // specific replacement character that UTF8ToWide() may replace the invalid
  // UTF-8 characters with because we want to allow that to change if the
  // implementation changes.
  EXPECT_EQ(0U, wide.find(L"before"));
  const wchar_t after_wide[] = L"after";
  EXPECT_EQ(wide.length() - (arraysize(after_wide) - 1), wide.find(after_wide));
}

// Below is adapted from https://chromium.googlesource.com/chromium/src/+/master/base/strings/utf_string_conversions_unittest.cc

// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the NOTICE file.

// The tests below from utf_string_conversions_unittest.cc check for this
// preprocessor symbol, so define it, as it is appropriate for Windows.
#define WCHAR_T_IS_UTF16
static_assert(sizeof(wchar_t) == 2, "wchar_t is not 2 bytes");

// The tests below from utf_string_conversions_unittest.cc call versions of
// UTF8ToWide() and WideToUTF8() that don't return success/failure, so these are
// stub implementations with that signature. These are just for testing and
// should not be moved to base because they assert/expect no errors which is
// probably not a good idea (or at least it is something that should be left
// up to the caller, not a base library).

static std::wstring UTF8ToWide(const std::string& utf8) {
  std::wstring utf16;
  EXPECT_TRUE(UTF8ToWide(utf8, &utf16));
  return utf16;
}

static std::string WideToUTF8(const std::wstring& utf16) {
  std::string utf8;
  EXPECT_TRUE(WideToUTF8(utf16, &utf8));
  return utf8;
}

namespace {

const wchar_t* const kConvertRoundtripCases[] = {
  L"Google Video",
  // "网页 图片 资讯更多 »"
  L"\x7f51\x9875\x0020\x56fe\x7247\x0020\x8d44\x8baf\x66f4\x591a\x0020\x00bb",
  //  "Παγκόσμιος Ιστός"
  L"\x03a0\x03b1\x03b3\x03ba\x03cc\x03c3\x03bc\x03b9"
  L"\x03bf\x03c2\x0020\x0399\x03c3\x03c4\x03cc\x03c2",
  // "Поиск страниц на русском"
  L"\x041f\x043e\x0438\x0441\x043a\x0020\x0441\x0442"
  L"\x0440\x0430\x043d\x0438\x0446\x0020\x043d\x0430"
  L"\x0020\x0440\x0443\x0441\x0441\x043a\x043e\x043c",
  // "전체서비스"
  L"\xc804\xccb4\xc11c\xbe44\xc2a4",

  // Test characters that take more than 16 bits. This will depend on whether
  // wchar_t is 16 or 32 bits.
#if defined(WCHAR_T_IS_UTF16)
  L"\xd800\xdf00",
  // ?????  (Mathematical Alphanumeric Symbols (U+011d40 - U+011d44 : A,B,C,D,E)
  L"\xd807\xdd40\xd807\xdd41\xd807\xdd42\xd807\xdd43\xd807\xdd44",
#elif defined(WCHAR_T_IS_UTF32)
  L"\x10300",
  // ?????  (Mathematical Alphanumeric Symbols (U+011d40 - U+011d44 : A,B,C,D,E)
  L"\x11d40\x11d41\x11d42\x11d43\x11d44",
#endif
};

}  // namespace

TEST(UTFStringConversionsTest, ConvertUTF8AndWide) {
  // we round-trip all the wide strings through UTF-8 to make sure everything
  // agrees on the conversion. This uses the stream operators to test them
  // simultaneously.
  for (size_t i = 0; i < arraysize(kConvertRoundtripCases); ++i) {
    std::ostringstream utf8;
    utf8 << WideToUTF8(kConvertRoundtripCases[i]);
    std::wostringstream wide;
    wide << UTF8ToWide(utf8.str());

    EXPECT_EQ(kConvertRoundtripCases[i], wide.str());
  }
}

TEST(UTFStringConversionsTest, ConvertUTF8AndWideEmptyString) {
  // An empty std::wstring should be converted to an empty std::string,
  // and vice versa.
  std::wstring wempty;
  std::string empty;
  EXPECT_EQ(empty, WideToUTF8(wempty));
  EXPECT_EQ(wempty, UTF8ToWide(empty));
}

TEST(UTFStringConversionsTest, ConvertUTF8ToWide) {
  struct UTF8ToWideCase {
    const char* utf8;
    const wchar_t* wide;
    bool success;
  } convert_cases[] = {
    // Regular UTF-8 input.
    {"\xe4\xbd\xa0\xe5\xa5\xbd", L"\x4f60\x597d", true},
    // Non-character is passed through.
    {"\xef\xbf\xbfHello", L"\xffffHello", true},
    // Truncated UTF-8 sequence.
    {"\xe4\xa0\xe5\xa5\xbd", L"\xfffd\x597d", false},
    // Truncated off the end.
    {"\xe5\xa5\xbd\xe4\xa0", L"\x597d\xfffd", false},
    // Non-shortest-form UTF-8.
    {"\xf0\x84\xbd\xa0\xe5\xa5\xbd", L"\xfffd\x597d", false},
    // This UTF-8 character decodes to a UTF-16 surrogate, which is illegal.
    // Note that for whatever reason, this test fails on Windows XP.
    {"\xed\xb0\x80", L"\xfffd", false},
    // Non-BMP characters. The second is a non-character regarded as valid.
    // The result will either be in UTF-16 or UTF-32.
#if defined(WCHAR_T_IS_UTF16)
    {"A\xF0\x90\x8C\x80z", L"A\xd800\xdf00z", true},
    {"A\xF4\x8F\xBF\xBEz", L"A\xdbff\xdffez", true},
#elif defined(WCHAR_T_IS_UTF32)
    {"A\xF0\x90\x8C\x80z", L"A\x10300z", true},
    {"A\xF4\x8F\xBF\xBEz", L"A\x10fffez", true},
#endif
  };

  for (size_t i = 0; i < arraysize(convert_cases); i++) {
    std::wstring converted;
    errno = 0;
    const bool success = UTF8ToWide(convert_cases[i].utf8,
                                    strlen(convert_cases[i].utf8),
                                    &converted);
    EXPECT_EQ(convert_cases[i].success, success);
    // The original test always compared expected and converted, but don't do
    // that because our implementation of UTF8ToWide() does not guarantee to
    // produce the same output in error situations.
    if (success) {
      std::wstring expected(convert_cases[i].wide);
      EXPECT_EQ(expected, converted);
    } else {
      EXPECT_EQ(EILSEQ, errno);
    }
  }

  // Manually test an embedded NULL.
  std::wstring converted;
  EXPECT_TRUE(UTF8ToWide("\00Z\t", 3, &converted));
  ASSERT_EQ(3U, converted.length());
  EXPECT_EQ(static_cast<wchar_t>(0), converted[0]);
  EXPECT_EQ('Z', converted[1]);
  EXPECT_EQ('\t', converted[2]);

  // Make sure that conversion replaces, not appends.
  EXPECT_TRUE(UTF8ToWide("B", 1, &converted));
  ASSERT_EQ(1U, converted.length());
  EXPECT_EQ('B', converted[0]);
}

#if defined(WCHAR_T_IS_UTF16)
// This test is only valid when wchar_t == UTF-16.
TEST(UTFStringConversionsTest, ConvertUTF16ToUTF8) {
  struct WideToUTF8Case {
    const wchar_t* utf16;
    const char* utf8;
    bool success;
  } convert_cases[] = {
    // Regular UTF-16 input.
    {L"\x4f60\x597d", "\xe4\xbd\xa0\xe5\xa5\xbd", true},
    // Test a non-BMP character.
    {L"\xd800\xdf00", "\xF0\x90\x8C\x80", true},
    // Non-characters are passed through.
    {L"\xffffHello", "\xEF\xBF\xBFHello", true},
    {L"\xdbff\xdffeHello", "\xF4\x8F\xBF\xBEHello", true},
    // The first character is a truncated UTF-16 character.
    // Note that for whatever reason, this test fails on Windows XP.
    {L"\xd800\x597d", "\xef\xbf\xbd\xe5\xa5\xbd",
#if (WINVER >= 0x0600)
    // Only Vista and later has a new API/flag that correctly returns false.
    false
#else
    true
#endif
    },
    // Truncated at the end.
    // Note that for whatever reason, this test fails on Windows XP.
    {L"\x597d\xd800", "\xe5\xa5\xbd\xef\xbf\xbd",
#if (WINVER >= 0x0600)
    // Only Vista and later has a new API/flag that correctly returns false.
    false
#else
    true
#endif
    },
  };

  for (size_t i = 0; i < arraysize(convert_cases); i++) {
    std::string converted;
    errno = 0;
    const bool success = WideToUTF8(convert_cases[i].utf16,
                                    wcslen(convert_cases[i].utf16),
                                    &converted);
    EXPECT_EQ(convert_cases[i].success, success);
    // The original test always compared expected and converted, but don't do
    // that because our implementation of WideToUTF8() does not guarantee to
    // produce the same output in error situations.
    if (success) {
      std::string expected(convert_cases[i].utf8);
      EXPECT_EQ(expected, converted);
    } else {
      EXPECT_EQ(EILSEQ, errno);
    }
  }
}

#elif defined(WCHAR_T_IS_UTF32)
// This test is only valid when wchar_t == UTF-32.
TEST(UTFStringConversionsTest, ConvertUTF32ToUTF8) {
  struct WideToUTF8Case {
    const wchar_t* utf32;
    const char* utf8;
    bool success;
  } convert_cases[] = {
    // Regular 16-bit input.
    {L"\x4f60\x597d", "\xe4\xbd\xa0\xe5\xa5\xbd", true},
    // Test a non-BMP character.
    {L"A\x10300z", "A\xF0\x90\x8C\x80z", true},
    // Non-characters are passed through.
    {L"\xffffHello", "\xEF\xBF\xBFHello", true},
    {L"\x10fffeHello", "\xF4\x8F\xBF\xBEHello", true},
    // Invalid Unicode code points.
    {L"\xfffffffHello", "\xEF\xBF\xBDHello", false},
    // The first character is a truncated UTF-16 character.
    {L"\xd800\x597d", "\xef\xbf\xbd\xe5\xa5\xbd", false},
    {L"\xdc01Hello", "\xef\xbf\xbdHello", false},
  };

  for (size_t i = 0; i < arraysize(convert_cases); i++) {
    std::string converted;
    EXPECT_EQ(convert_cases[i].success,
              WideToUTF8(convert_cases[i].utf32,
                         wcslen(convert_cases[i].utf32),
                         &converted));
    std::string expected(convert_cases[i].utf8);
    EXPECT_EQ(expected, converted);
  }
}
#endif  // defined(WCHAR_T_IS_UTF32)

// The test below uses these types and functions, so just do enough to get the
// test running.
typedef wchar_t char16;
typedef std::wstring string16;

template<typename T>
static void* WriteInto(T* t, size_t size) {
  // std::(w)string::resize() already includes space for a NULL terminator.
  t->resize(size - 1);
  return &(*t)[0];
}

// A stub implementation that calls a helper from above, just to get the test
// below working. This is just for testing and should not be moved to base
// because this ignores errors which is probably not a good idea, plus it takes
// a string16 type which we don't really have.
static std::string UTF16ToUTF8(const string16& utf16) {
  return WideToUTF8(utf16);
}

TEST(UTFStringConversionsTest, ConvertMultiString) {
  static char16 multi16[] = {
    'f', 'o', 'o', '\0',
    'b', 'a', 'r', '\0',
    'b', 'a', 'z', '\0',
    '\0'
  };
  static char multi[] = {
    'f', 'o', 'o', '\0',
    'b', 'a', 'r', '\0',
    'b', 'a', 'z', '\0',
    '\0'
  };
  string16 multistring16;
  memcpy(WriteInto(&multistring16, arraysize(multi16)), multi16,
                   sizeof(multi16));
  EXPECT_EQ(arraysize(multi16) - 1, multistring16.length());
  std::string expected;
  memcpy(WriteInto(&expected, arraysize(multi)), multi, sizeof(multi));
  EXPECT_EQ(arraysize(multi) - 1, expected.length());
  const std::string& converted = UTF16ToUTF8(multistring16);
  EXPECT_EQ(arraysize(multi) - 1, converted.length());
  EXPECT_EQ(expected, converted);
}

// The tests below from sys_string_conversions_unittest.cc call SysWideToUTF8()
// and SysUTF8ToWide(), so these are stub implementations that call the helpers
// above. These are just for testing and should not be moved to base because
// they ignore errors which is probably not a good idea.

static std::string SysWideToUTF8(const std::wstring& utf16) {
  return WideToUTF8(utf16);
}

static std::wstring SysUTF8ToWide(const std::string& utf8) {
  return UTF8ToWide(utf8);
}

// Below is adapted from https://chromium.googlesource.com/chromium/src/+/master/base/strings/sys_string_conversions_unittest.cc

// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifdef WCHAR_T_IS_UTF32
static const std::wstring kSysWideOldItalicLetterA = L"\x10300";
#else
static const std::wstring kSysWideOldItalicLetterA = L"\xd800\xdf00";
#endif

TEST(SysStrings, SysWideToUTF8) {
  EXPECT_EQ("Hello, world", SysWideToUTF8(L"Hello, world"));
  EXPECT_EQ("\xe4\xbd\xa0\xe5\xa5\xbd", SysWideToUTF8(L"\x4f60\x597d"));

  // >16 bits
  EXPECT_EQ("\xF0\x90\x8C\x80", SysWideToUTF8(kSysWideOldItalicLetterA));

  // Error case. When Windows finds a UTF-16 character going off the end of
  // a string, it just converts that literal value to UTF-8, even though this
  // is invalid.
  //
  // This is what XP does, but Vista has different behavior, so we don't bother
  // verifying it:
  // EXPECT_EQ("\xE4\xBD\xA0\xED\xA0\x80zyxw",
  //           SysWideToUTF8(L"\x4f60\xd800zyxw"));

  // Test embedded NULLs.
  std::wstring wide_null(L"a");
  wide_null.push_back(0);
  wide_null.push_back('b');

  std::string expected_null("a");
  expected_null.push_back(0);
  expected_null.push_back('b');

  EXPECT_EQ(expected_null, SysWideToUTF8(wide_null));
}

TEST(SysStrings, SysUTF8ToWide) {
  EXPECT_EQ(L"Hello, world", SysUTF8ToWide("Hello, world"));
  EXPECT_EQ(L"\x4f60\x597d", SysUTF8ToWide("\xe4\xbd\xa0\xe5\xa5\xbd"));
  // >16 bits
  EXPECT_EQ(kSysWideOldItalicLetterA, SysUTF8ToWide("\xF0\x90\x8C\x80"));

  // Error case. When Windows finds an invalid UTF-8 character, it just skips
  // it. This seems weird because it's inconsistent with the reverse conversion.
  //
  // This is what XP does, but Vista has different behavior, so we don't bother
  // verifying it:
  // EXPECT_EQ(L"\x4f60zyxw", SysUTF8ToWide("\xe4\xbd\xa0\xe5\xa5zyxw"));

  // Test embedded NULLs.
  std::string utf8_null("a");
  utf8_null.push_back(0);
  utf8_null.push_back('b');

  std::wstring expected_null(L"a");
  expected_null.push_back(0);
  expected_null.push_back('b');

  EXPECT_EQ(expected_null, SysUTF8ToWide(utf8_null));
}

TEST(UTF8PathToWindowsLongPathTest, DontAddPrefixIfShorterThanMaxPath) {
  std::string utf8 = "c:\\mypath\\myfile.txt";

  std::wstring wide;
  EXPECT_TRUE(UTF8PathToWindowsLongPath(utf8.c_str(), &wide));

  EXPECT_EQ(std::string::npos, wide.find(LR"(\\?\)"));
}

TEST(UTF8PathToWindowsLongPathTest, AddPrefixIfLongerThanMaxPath) {
  std::string utf8 = "c:\\mypath";
  while (utf8.length() < 300 /* MAX_PATH is 260 */) {
    utf8 += "\\mypathsegment";
  }

  std::wstring wide;
  EXPECT_TRUE(UTF8PathToWindowsLongPath(utf8.c_str(), &wide));

  EXPECT_EQ(0U, wide.find(LR"(\\?\)"));
  EXPECT_EQ(std::string::npos, wide.find(L"/"));
}

TEST(UTF8PathToWindowsLongPathTest, AddPrefixAndFixSeparatorsIfLongerThanMaxPath) {
  std::string utf8 = "c:/mypath";
  while (utf8.length() < 300 /* MAX_PATH is 260 */) {
    utf8 += "/mypathsegment";
  }

  std::wstring wide;
  EXPECT_TRUE(UTF8PathToWindowsLongPath(utf8.c_str(), &wide));

  EXPECT_EQ(0U, wide.find(LR"(\\?\)"));
  EXPECT_EQ(std::string::npos, wide.find(L"/"));
}

namespace utf8 {

TEST(Utf8FilesTest, CanCreateOpenAndDeleteFileWithLongPath) {
  TemporaryDir td;

  // Create long directory path
  std::string utf8 = td.path;
  while (utf8.length() < 300 /* MAX_PATH is 260 */) {
    utf8 += "\\mypathsegment";
    EXPECT_EQ(0, mkdir(utf8.c_str(), 0));
  }

  // Create file
  utf8 += "\\test-file.bin";
  int flags = O_WRONLY | O_CREAT | O_TRUNC | O_BINARY;
  int mode = 0666;
  android::base::unique_fd fd(open(utf8.c_str(), flags, mode));
  EXPECT_NE(-1, fd.get());

  // Close file
  fd.reset();
  EXPECT_EQ(-1, fd.get());

  // Open file with fopen
  FILE* file = fopen(utf8.c_str(), "rb");
  EXPECT_NE(nullptr, file);

  if (file) {
    fclose(file);
  }

  // Delete file
  EXPECT_EQ(0, unlink(utf8.c_str()));
}

}  // namespace utf8
}  // namespace base
}  // namespace android
