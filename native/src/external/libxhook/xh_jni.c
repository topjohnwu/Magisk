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

#include <jni.h>
#include "xhook.h"

#define JNI_API_DEF(f) Java_com_qiyi_xhook_NativeHandler_##f

JNIEXPORT jint JNI_API_DEF(refresh)(JNIEnv *env, jobject obj, jboolean async)
{
    (void)env;
    (void)obj;

    return xhook_refresh(async ? 1 : 0);
}

JNIEXPORT void JNI_API_DEF(clear)(JNIEnv *env, jobject obj)
{
    (void)env;
    (void)obj;

    xhook_clear();
}

JNIEXPORT void JNI_API_DEF(enableDebug)(JNIEnv *env, jobject obj, jboolean flag)
{
    (void)env;
    (void)obj;

    xhook_enable_debug(flag ? 1 : 0);
}

JNIEXPORT void JNI_API_DEF(enableSigSegvProtection)(JNIEnv *env, jobject obj, jboolean flag)
{
    (void)env;
    (void)obj;

    xhook_enable_sigsegv_protection(flag ? 1 : 0);
}
