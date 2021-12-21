//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef SUPPORT_NASTY_MACROS_HPP
#define SUPPORT_NASTY_MACROS_HPP

#define NASTY_MACRO This should not be expanded!!!
#define _A NASTY_MACRO
#define _B NASTY_MACRO
#define _C NASTY_MACRO
#define _D NASTY_MACRO
#define _E NASTY_MACRO
#define _F NASTY_MACRO
#define _G NASTY_MACRO
#define _H NASTY_MACRO
#define _I NASTY_MACRO
#define _J NASTY_MACRO
#define _K NASTY_MACRO
#define _L NASTY_MACRO
// Because FreeBSD uses _M in its <sys/types.h>, and it is hard to avoid
// including that header, only define _M for other operating systems.
#ifndef __FreeBSD__
#define _M NASTY_MACRO
#endif
#define _N NASTY_MACRO
#define _O NASTY_MACRO
#define _P NASTY_MACRO
#define _Q NASTY_MACRO
#define _R NASTY_MACRO
#define _S NASTY_MACRO
#define _T NASTY_MACRO
#define _U NASTY_MACRO
#define _V NASTY_MACRO
#define _W NASTY_MACRO
#define _X NASTY_MACRO
#define _Y NASTY_MACRO
#define _Z NASTY_MACRO

// tchar.h defines these macros on Windows.
#define _UI   NASTY_MACRO
#define _PUC  NASTY_MACRO
#define _CPUC NASTY_MACRO
#define _PC   NASTY_MACRO
#define _CRPC NASTY_MACRO
#define _CPC  NASTY_MACRO

// yvals.h on MINGW defines this macro
#define _C2 NASTY_MACRO

// Test that libc++ doesn't use names reserved by WIN32 API Macros.
// NOTE: Obviously we can only define these on non-windows platforms.
#ifndef _WIN32
#define __deallocate NASTY_MACRO
#define __out NASTY_MACRO
#endif

#define __output NASTY_MACRO
#define __input NASTY_MACRO

#endif // SUPPORT_NASTY_MACROS_HPP
