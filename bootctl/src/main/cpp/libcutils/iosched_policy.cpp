/*
** Copyright 2007, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include <cutils/iosched_policy.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(__ANDROID__)
#define IOPRIO_WHO_PROCESS (1)
#define IOPRIO_CLASS_SHIFT (13)
#include <sys/syscall.h>
#define __android_unused
#else
#define __android_unused __attribute__((__unused__))
#endif

int android_set_ioprio(int pid __android_unused, IoSchedClass clazz __android_unused, int ioprio __android_unused) {
#if defined(__ANDROID__)
    if (syscall(SYS_ioprio_set, IOPRIO_WHO_PROCESS, pid, ioprio | (clazz << IOPRIO_CLASS_SHIFT))) {
        return -1;
    }
#endif
    return 0;
}

int android_get_ioprio(int pid __android_unused, IoSchedClass *clazz, int *ioprio) {
#if defined(__ANDROID__)
    int rc;

    if ((rc = syscall(SYS_ioprio_get, IOPRIO_WHO_PROCESS, pid)) < 0) {
        return -1;
    }

    *clazz = static_cast<IoSchedClass>(rc >> IOPRIO_CLASS_SHIFT);
    *ioprio = (rc & 0xff);
#else
    *clazz = IoSchedClass_NONE;
    *ioprio = 0;
#endif
    return 0;
}
