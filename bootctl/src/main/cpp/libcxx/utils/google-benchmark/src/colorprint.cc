// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "colorprint.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

#include "check.h"
#include "internal_macros.h"

#ifdef BENCHMARK_OS_WINDOWS
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif  // BENCHMARK_OS_WINDOWS

namespace benchmark {
namespace {
#ifdef BENCHMARK_OS_WINDOWS
typedef WORD PlatformColorCode;
#else
typedef const char* PlatformColorCode;
#endif

PlatformColorCode GetPlatformColorCode(LogColor color) {
#ifdef BENCHMARK_OS_WINDOWS
  switch (color) {
    case COLOR_RED:
      return FOREGROUND_RED;
    case COLOR_GREEN:
      return FOREGROUND_GREEN;
    case COLOR_YELLOW:
      return FOREGROUND_RED | FOREGROUND_GREEN;
    case COLOR_BLUE:
      return FOREGROUND_BLUE;
    case COLOR_MAGENTA:
      return FOREGROUND_BLUE | FOREGROUND_RED;
    case COLOR_CYAN:
      return FOREGROUND_BLUE | FOREGROUND_GREEN;
    case COLOR_WHITE:  // fall through to default
    default:
      return 0;
  }
#else
  switch (color) {
    case COLOR_RED:
      return "1";
    case COLOR_GREEN:
      return "2";
    case COLOR_YELLOW:
      return "3";
    case COLOR_BLUE:
      return "4";
    case COLOR_MAGENTA:
      return "5";
    case COLOR_CYAN:
      return "6";
    case COLOR_WHITE:
      return "7";
    default:
      return nullptr;
  };
#endif
}

}  // end namespace

std::string FormatString(const char* msg, va_list args) {
  // we might need a second shot at this, so pre-emptivly make a copy
  va_list args_cp;
  va_copy(args_cp, args);

  std::size_t size = 256;
  char local_buff[256];
  auto ret = vsnprintf(local_buff, size, msg, args_cp);

  va_end(args_cp);

  // currently there is no error handling for failure, so this is hack.
  CHECK(ret >= 0);

  if (ret == 0)  // handle empty expansion
    return {};
  else if (static_cast<size_t>(ret) < size)
    return local_buff;
  else {
    // we did not provide a long enough buffer on our first attempt.
    size = (size_t)ret + 1;  // + 1 for the null byte
    std::unique_ptr<char[]> buff(new char[size]);
    ret = vsnprintf(buff.get(), size, msg, args);
    CHECK(ret > 0 && ((size_t)ret) < size);
    return buff.get();
  }
}

std::string FormatString(const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  auto tmp = FormatString(msg, args);
  va_end(args);
  return tmp;
}

void ColorPrintf(std::ostream& out, LogColor color, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  ColorPrintf(out, color, fmt, args);
  va_end(args);
}

void ColorPrintf(std::ostream& out, LogColor color, const char* fmt,
                 va_list args) {
#ifdef BENCHMARK_OS_WINDOWS
  ((void)out);  // suppress unused warning

  const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

  // Gets the current text color.
  CONSOLE_SCREEN_BUFFER_INFO buffer_info;
  GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
  const WORD old_color_attrs = buffer_info.wAttributes;

  // We need to flush the stream buffers into the console before each
  // SetConsoleTextAttribute call lest it affect the text that is already
  // printed but has not yet reached the console.
  fflush(stdout);
  SetConsoleTextAttribute(stdout_handle,
                          GetPlatformColorCode(color) | FOREGROUND_INTENSITY);
  vprintf(fmt, args);

  fflush(stdout);
  // Restores the text color.
  SetConsoleTextAttribute(stdout_handle, old_color_attrs);
#else
  const char* color_code = GetPlatformColorCode(color);
  if (color_code) out << FormatString("\033[0;3%sm", color_code);
  out << FormatString(fmt, args) << "\033[m";
#endif
}

bool IsColorTerminal() {
#if BENCHMARK_OS_WINDOWS
  // On Windows the TERM variable is usually not set, but the
  // console there does support colors.
  return 0 != _isatty(_fileno(stdout));
#else
  // On non-Windows platforms, we rely on the TERM variable. This list of
  // supported TERM values is copied from Google Test:
  // <https://github.com/google/googletest/blob/master/googletest/src/gtest.cc#L2925>.
  const char* const SUPPORTED_TERM_VALUES[] = {
      "xterm",         "xterm-color",     "xterm-256color",
      "screen",        "screen-256color", "tmux",
      "tmux-256color", "rxvt-unicode",    "rxvt-unicode-256color",
      "linux",         "cygwin",
  };

  const char* const term = getenv("TERM");

  bool term_supports_color = false;
  for (const char* candidate : SUPPORTED_TERM_VALUES) {
    if (term && 0 == strcmp(term, candidate)) {
      term_supports_color = true;
      break;
    }
  }

  return 0 != isatty(fileno(stdout)) && term_supports_color;
#endif  // BENCHMARK_OS_WINDOWS
}

}  // end namespace benchmark
