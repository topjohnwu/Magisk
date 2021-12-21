/*
 * Copyright 2011, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cutils/android_reboot.h>

#include <stdio.h>
#include <stdlib.h>

#include <cutils/properties.h>

#define TAG "android_reboot"

int android_reboot(unsigned cmd, int /*flags*/, const char* arg) {
    int ret;
    const char* restart_cmd = NULL;
    char* prop_value;

    switch (cmd) {
        case ANDROID_RB_RESTART:  // deprecated
        case ANDROID_RB_RESTART2:
            restart_cmd = "reboot";
            break;
        case ANDROID_RB_POWEROFF:
            restart_cmd = "shutdown";
            break;
        case ANDROID_RB_THERMOFF:
            restart_cmd = "shutdown,thermal";
            break;
    }
    if (!restart_cmd) return -1;
    if (arg && arg[0]) {
        ret = asprintf(&prop_value, "%s,%s", restart_cmd, arg);
    } else {
        ret = asprintf(&prop_value, "%s", restart_cmd);
    }
    if (ret < 0) return -1;
    ret = property_set(ANDROID_RB_PROPERTY, prop_value);
    free(prop_value);
    return ret;
}
