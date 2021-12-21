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

#include <cutils/partition_utils.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mount.h> /* for BLKGETSIZE */
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cutils/properties.h>

static int only_one_char(uint8_t *buf, int len, uint8_t c)
{
    int i, ret;

    ret = 1;
    for (i=0; i<len; i++) {
        if (buf[i] != c) {
            ret = 0;
            break;
        }
    }
    return ret;
}

int partition_wiped(const char* source) {
    uint8_t buf[4096];
    int fd, ret;

    if ((fd = open(source, O_RDONLY)) < 0) {
        return 0;
    }

    ret = read(fd, buf, sizeof(buf));
    close(fd);

    if (ret != sizeof(buf)) {
        return 0;
    }

    /* Check for all zeros */
    if (only_one_char(buf, sizeof(buf), 0)) {
       return 1;
    }

    /* Check for all ones */
    if (only_one_char(buf, sizeof(buf), 0xff)) {
       return 1;
    }

    return 0;
}
