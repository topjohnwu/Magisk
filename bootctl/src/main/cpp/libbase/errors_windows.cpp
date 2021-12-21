/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include "android-base/errors.h"

#include <windows.h>

#include "android-base/stringprintf.h"
#include "android-base/strings.h"
#include "android-base/utf8.h"

// A Windows error code is a DWORD. It's simpler to use an int error code for
// both Unix and Windows if possible, but if this fails we'll need a different
// function signature for each.
static_assert(sizeof(int) >= sizeof(DWORD),
              "Windows system error codes are too large to fit in an int.");

namespace android {
namespace base {

static constexpr DWORD kErrorMessageBufferSize = 256;

std::string SystemErrorCodeToString(int int_error_code) {
  WCHAR msgbuf[kErrorMessageBufferSize];
  DWORD error_code = int_error_code;
  DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
  DWORD len = FormatMessageW(flags, nullptr, error_code, 0, msgbuf,
                             kErrorMessageBufferSize, nullptr);
  if (len == 0) {
    return android::base::StringPrintf(
        "Error %lu while retrieving message for error %lu", GetLastError(),
        error_code);
  }

  // Convert UTF-16 to UTF-8.
  std::string msg;
  if (!android::base::WideToUTF8(msgbuf, &msg)) {
    return android::base::StringPrintf(
        "Error %lu while converting message for error %lu from UTF-16 to UTF-8",
        GetLastError(), error_code);
  }

  // Messages returned by the system end with line breaks.
  msg = android::base::Trim(msg);

  // There are many Windows error messages compared to POSIX, so include the
  // numeric error code for easier, quicker, accurate identification. Use
  // decimal instead of hex because there are decimal ranges like 10000-11999
  // for Winsock.
  android::base::StringAppendF(&msg, " (%lu)", error_code);
  return msg;
}

}  // namespace base
}  // namespace android
