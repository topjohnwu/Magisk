// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef SUPPORT_SET_WINDOWS_CRT_REPORT_MODE_H
#define SUPPORT_SET_WINDOWS_CRT_REPORT_MODE_H

#ifndef _DEBUG
#error _DEBUG must be defined when using this header
#endif

#ifndef _WIN32
#error This header can only be used when targeting Windows
#endif

#include <crtdbg.h>

// On Windows in debug builds the default assertion handler opens a new dialog
// window which must be dismissed manually by the user. This function overrides
// that setting and instead changes the assertion handler to log to stderr
// instead.
inline int init_crt_report_mode() {
  _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
  _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
  _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
  return 0;
}

static int init_crt_anchor = init_crt_report_mode();

#endif // SUPPORT_SET_WINDOWS_CRT_REPORT_MODE_H
