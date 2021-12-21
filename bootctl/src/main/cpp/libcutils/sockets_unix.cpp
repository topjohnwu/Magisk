/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include <cutils/sockets.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "android_get_control_env.h"

int socket_close(int sock) {
    return close(sock);
}

ssize_t socket_send_buffers(cutils_socket_t sock,
                            const cutils_socket_buffer_t* buffers,
                            size_t num_buffers) {
    if (num_buffers > SOCKET_SEND_BUFFERS_MAX_BUFFERS) {
        return -1;
    }

    iovec iovec_buffers[SOCKET_SEND_BUFFERS_MAX_BUFFERS];
    for (size_t i = 0; i < num_buffers; ++i) {
        // It's safe to cast away const here; iovec declares non-const
        // void* because it's used for both send and receive, but since
        // we're only sending, the data won't be modified.
        iovec_buffers[i].iov_base = const_cast<void*>(buffers[i].data);
        iovec_buffers[i].iov_len = buffers[i].length;
    }

    return writev(sock, iovec_buffers, num_buffers);
}

#if defined(__ANDROID__)
int android_get_control_socket(const char* name) {
    int fd = __android_get_control_from_env(ANDROID_SOCKET_ENV_PREFIX, name);

    if (fd < 0) return fd;

    // Compare to UNIX domain socket name, must match!
    struct sockaddr_un addr;
    socklen_t addrlen = sizeof(addr);
    int ret = getsockname(fd, (struct sockaddr*)&addr, &addrlen);
    if (ret < 0) return -1;

    constexpr char prefix[] = ANDROID_SOCKET_DIR "/";
    constexpr size_t prefix_size = sizeof(prefix) - sizeof('\0');
    if ((strncmp(addr.sun_path, prefix, prefix_size) == 0) &&
        (strcmp(addr.sun_path + prefix_size, name) == 0)) {
        // It is what we think it is
        return fd;
    }
    return -1;
}
#else
int android_get_control_socket(const char*) {
    return -1;
}
#endif
