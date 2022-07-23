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

#ifndef XHOOK_H
#define XHOOK_H 1

#ifdef __cplusplus
extern "C" {
#endif

#define XHOOK_EXPORT __attribute__((visibility("default")))

int xhook_register(const char *pathname_regex_str, const char *symbol,
                   void *new_func, void **old_func) XHOOK_EXPORT;

int xhook_ignore(const char *pathname_regex_str, const char *symbol) XHOOK_EXPORT;

int xhook_refresh(int async) XHOOK_EXPORT;

void xhook_clear() XHOOK_EXPORT;

void xhook_enable_debug(int flag) XHOOK_EXPORT;

void xhook_enable_sigsegv_protection(int flag) XHOOK_EXPORT;

#ifdef __cplusplus
}
#endif

#endif
