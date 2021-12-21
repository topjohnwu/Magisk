// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// test <errno.h>

#include <errno.h>

#ifndef EDOM
#error EDOM not defined
#endif

#ifndef EILSEQ
#error EILSEQ not defined
#endif

#ifndef ERANGE
#error ERANGE not defined
#endif

#ifndef errno
#error errno not defined
#endif

int main()
{
}
