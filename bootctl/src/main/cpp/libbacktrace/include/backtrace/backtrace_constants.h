/*
 * Copyright (C) 2014 The Android Open Source Project
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

#ifndef _BACKTRACE_BACKTRACE_CONSTANTS_H
#define _BACKTRACE_BACKTRACE_CONSTANTS_H

// When the pid to be traced is set to this value, then trace the current
// process. If the tid value is not BACKTRACE_NO_TID, then the specified
// thread from the current process will be traced.
#define BACKTRACE_CURRENT_PROCESS (-1)
// When the tid to be traced is set to this value, then trace the specified
// current thread of the specified pid.
#define BACKTRACE_CURRENT_THREAD (-1)

#define MAX_BACKTRACE_FRAMES 256

#endif // _BACKTRACE_BACKTRACE_CONSTANTS_H
