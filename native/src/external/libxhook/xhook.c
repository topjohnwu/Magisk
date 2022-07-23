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

#include "xh_core.h"
#include "xhook.h"

int xhook_register(const char *pathname_regex_str, const char *symbol,
                   void *new_func, void **old_func)
{
    return xh_core_register(pathname_regex_str, symbol, new_func, old_func);
}

int xhook_ignore(const char *pathname_regex_str, const char *symbol)
{
    return xh_core_ignore(pathname_regex_str, symbol);
}

int xhook_refresh(int async)
{
    return xh_core_refresh(async);
}

void xhook_clear()
{
    return xh_core_clear();
}

void xhook_enable_debug(int flag)
{
    return xh_core_enable_debug(flag);
}

void xhook_enable_sigsegv_protection(int flag)
{
    return xh_core_enable_sigsegv_protection(flag);
}
