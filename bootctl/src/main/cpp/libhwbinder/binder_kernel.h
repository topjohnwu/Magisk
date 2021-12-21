/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_BINDER_KERNEL_H
#define ANDROID_HARDWARE_BINDER_KERNEL_H

// TODO(b/31559095): bionic on host
#ifndef __ANDROID__
#define __packed __attribute__((__packed__))
#endif

#include <sys/ioctl.h>
#include <linux/android/binder.h>

#ifndef BR_ONEWAY_SPAM_SUSPECT
// Temporary definition of BR_ONEWAY_SPAM_SUSPECT. For production
// this will come from UAPI binder.h
#define BR_ONEWAY_SPAM_SUSPECT _IO('r', 19)
#endif //BR_ONEWAY_SPAM_SUSPECT

#ifndef BINDER_ENABLE_ONEWAY_SPAM_DETECTION
/*
 * Temporary definitions for oneway spam detection support. For the final version
 * these will be defined in the UAPI binder.h file from upstream kernel.
 */
#define BINDER_ENABLE_ONEWAY_SPAM_DETECTION _IOW('b', 16, __u32)
#endif //BINDER_ENABLE_ONEWAY_SPAM_DETECTION

#endif // ANDROID_HARDWARE_BINDER_KERNEL_H
