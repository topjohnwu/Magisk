// Copyright (c) 2018-present, iQIYI, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

// Created by caikelun on 2018-04-11.

#include "xh_version.h"

#define XH_VERSION_MAJOR 1
#define XH_VERSION_MINOR 2
#define XH_VERSION_EXTRA 0

#define XH_VERSION ((XH_VERSION_MAJOR << 16) | (XH_VERSION_MINOR <<  8) | (XH_VERSION_EXTRA))

#define XH_VERSION_TO_STR_HELPER(x) #x
#define XH_VERSION_TO_STR(x) XH_VERSION_TO_STR_HELPER(x)

#define XH_VERSION_STR XH_VERSION_TO_STR(XH_VERSION_MAJOR) "." \
                       XH_VERSION_TO_STR(XH_VERSION_MINOR) "." \
                       XH_VERSION_TO_STR(XH_VERSION_EXTRA)

#if defined(__arm__)
#define XH_VERSION_ARCH "arm"
#elif defined(__aarch64__)
#define XH_VERSION_ARCH "aarch64"
#elif defined(__i386__)
#define XH_VERSION_ARCH "x86"
#elif defined(__x86_64__)
#define XH_VERSION_ARCH "x86_64"
#else
#define XH_VERSION_ARCH "unknown"
#endif

#define XH_VERSION_STR_FULL "libxhook "XH_VERSION_STR" ("XH_VERSION_ARCH")"

unsigned int xh_version()
{
    return XH_VERSION;
}

const char *xh_version_str()
{
    return XH_VERSION_STR;
}

const char *xh_version_str_full()
{
    return XH_VERSION_STR_FULL;
}
