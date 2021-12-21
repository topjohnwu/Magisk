/*
 * Copyright (C) 2007 The Android Open Source Project
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

#pragma once

#include  <sys/types.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// Deprecated: use android::base::GetThreadId instead, which doesn't truncate on Mac/Windows.
//
#if !defined(__GLIBC__) || __GLIBC__ >= 2 && __GLIBC_MINOR__ < 32
extern pid_t gettid();
#endif

#ifdef __cplusplus
}
#endif
