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

#include <windows.h>

#include "android-base/utf8.h"

#include <fcntl.h>
#include <stdio.h>

#include <algorithm>
#include <string>

#include "android-base/logging.h"

namespace android {
namespace base {

// Helper to set errno based on GetLastError() after WideCharToMultiByte()/MultiByteToWideChar().
static void SetErrnoFromLastError() {
  switch (GetLastError()) {
    case ERROR_NO_UNICODE_TRANSLATION:
      errno = EILSEQ;
      break;
    default:
      errno = EINVAL;
      break;
  }
}

bool WideToUTF8(const wchar_t* utf16, const size_t size, std::string* utf8) {
  utf8->clear();

  if (size == 0) {
    return true;
  }

  // TODO: Consider using std::wstring_convert once libcxx is supported on
  // Windows.

  // Only Vista or later has this flag that causes WideCharToMultiByte() to
  // return an error on invalid characters.
  const DWORD flags =
#if (WINVER >= 0x0600)
    WC_ERR_INVALID_CHARS;
#else
    0;
#endif

  const int chars_required = WideCharToMultiByte(CP_UTF8, flags, utf16, size,
                                                 NULL, 0, NULL, NULL);
  if (chars_required <= 0) {
    SetErrnoFromLastError();
    return false;
  }

  // This could potentially throw a std::bad_alloc exception.
  utf8->resize(chars_required);

  const int result = WideCharToMultiByte(CP_UTF8, flags, utf16, size,
                                         &(*utf8)[0], chars_required, NULL,
                                         NULL);
  if (result != chars_required) {
    SetErrnoFromLastError();
    CHECK_LE(result, chars_required) << "WideCharToMultiByte wrote " << result
        << " chars to buffer of " << chars_required << " chars";
    utf8->clear();
    return false;
  }

  return true;
}

bool WideToUTF8(const wchar_t* utf16, std::string* utf8) {
  // Compute string length of NULL-terminated string with wcslen().
  return WideToUTF8(utf16, wcslen(utf16), utf8);
}

bool WideToUTF8(const std::wstring& utf16, std::string* utf8) {
  // Use the stored length of the string which allows embedded NULL characters
  // to be converted.
  return WideToUTF8(utf16.c_str(), utf16.length(), utf8);
}

// Internal helper function that takes MultiByteToWideChar() flags.
static bool UTF8ToWideWithFlags(const char* utf8, const size_t size, std::wstring* utf16,
                                const DWORD flags) {
  utf16->clear();

  if (size == 0) {
    return true;
  }

  // TODO: Consider using std::wstring_convert once libcxx is supported on
  // Windows.
  const int chars_required = MultiByteToWideChar(CP_UTF8, flags, utf8, size,
                                                 NULL, 0);
  if (chars_required <= 0) {
    SetErrnoFromLastError();
    return false;
  }

  // This could potentially throw a std::bad_alloc exception.
  utf16->resize(chars_required);

  const int result = MultiByteToWideChar(CP_UTF8, flags, utf8, size,
                                         &(*utf16)[0], chars_required);
  if (result != chars_required) {
    SetErrnoFromLastError();
    CHECK_LE(result, chars_required) << "MultiByteToWideChar wrote " << result
        << " chars to buffer of " << chars_required << " chars";
    utf16->clear();
    return false;
  }

  return true;
}

bool UTF8ToWide(const char* utf8, const size_t size, std::wstring* utf16) {
  // If strictly interpreting as UTF-8 succeeds, return success.
  if (UTF8ToWideWithFlags(utf8, size, utf16, MB_ERR_INVALID_CHARS)) {
    return true;
  }

  const int saved_errno = errno;

  // Fallback to non-strict interpretation, allowing invalid characters and
  // converting as best as possible, and return false to signify a problem.
  (void)UTF8ToWideWithFlags(utf8, size, utf16, 0);
  errno = saved_errno;
  return false;
}

bool UTF8ToWide(const char* utf8, std::wstring* utf16) {
  // Compute string length of NULL-terminated string with strlen().
  return UTF8ToWide(utf8, strlen(utf8), utf16);
}

bool UTF8ToWide(const std::string& utf8, std::wstring* utf16) {
  // Use the stored length of the string which allows embedded NULL characters
  // to be converted.
  return UTF8ToWide(utf8.c_str(), utf8.length(), utf16);
}

static bool isDriveLetter(wchar_t c) {
  return (c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z');
}

bool UTF8PathToWindowsLongPath(const char* utf8, std::wstring* utf16) {
  if (!UTF8ToWide(utf8, utf16)) {
    return false;
  }
  // Note: Although most Win32 File I/O API are limited to MAX_PATH (260
  //       characters), the CreateDirectory API is limited to 248 characters.
  if (utf16->length() >= 248) {
    // If path is of the form "x:\" or "x:/"
    if (isDriveLetter((*utf16)[0]) && (*utf16)[1] == L':' &&
        ((*utf16)[2] == L'\\' || (*utf16)[2] == L'/')) {
      // Append long path prefix, and make sure there are no unix-style
      // separators to ensure a fully compliant Win32 long path string.
      utf16->insert(0, LR"(\\?\)");
      std::replace(utf16->begin(), utf16->end(), L'/', L'\\');
    }
  }
  return true;
}

// Versions of standard library APIs that support UTF-8 strings.
namespace utf8 {

FILE* fopen(const char* name, const char* mode) {
  std::wstring name_utf16;
  if (!UTF8PathToWindowsLongPath(name, &name_utf16)) {
    return nullptr;
  }

  std::wstring mode_utf16;
  if (!UTF8ToWide(mode, &mode_utf16)) {
    return nullptr;
  }

  return _wfopen(name_utf16.c_str(), mode_utf16.c_str());
}

int mkdir(const char* name, mode_t) {
  std::wstring name_utf16;
  if (!UTF8PathToWindowsLongPath(name, &name_utf16)) {
    return -1;
  }

  return _wmkdir(name_utf16.c_str());
}

int open(const char* name, int flags, ...) {
  std::wstring name_utf16;
  if (!UTF8PathToWindowsLongPath(name, &name_utf16)) {
    return -1;
  }

  int mode = 0;
  if ((flags & O_CREAT) != 0) {
    va_list args;
    va_start(args, flags);
    mode = va_arg(args, int);
    va_end(args);
  }

  return _wopen(name_utf16.c_str(), flags, mode);
}

int unlink(const char* name) {
  std::wstring name_utf16;
  if (!UTF8PathToWindowsLongPath(name, &name_utf16)) {
    return -1;
  }

  return _wunlink(name_utf16.c_str());
}

}  // namespace utf8
}  // namespace base
}  // namespace android
