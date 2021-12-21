//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef SUPPORT_DISABLE_MISSING_BRACES_WARNING_H
#define SUPPORT_DISABLE_MISSING_BRACES_WARNING_H

// std::array is explicitly allowed to be initialized with A a = { init-list };.
// Disable the missing braces warning for this reason.
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wmissing-braces"
#elif defined(__clang__)
#pragma clang diagnostic ignored "-Wmissing-braces"
#endif

#endif // SUPPORT_DISABLE_MISSING_BRACES_WARNING_H
