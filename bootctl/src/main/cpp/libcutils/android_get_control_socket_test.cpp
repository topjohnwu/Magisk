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

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>

#include <cutils/sockets.h>
#include <gtest/gtest.h>

#ifndef SOCK_NONBLOCK
#define SOCK_NONBLOCK 0
#endif

#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC 0
#endif

TEST(SocketsTest, android_get_control_socket) {
    static const char key[] = ANDROID_SOCKET_ENV_PREFIX "SocketsTest_android_get_control_socket";
    static const char* name = key + strlen(ANDROID_SOCKET_ENV_PREFIX);

    EXPECT_EQ(unsetenv(key), 0);
    EXPECT_EQ(android_get_control_socket(name), -1);

    int fd;
    ASSERT_GE(fd = socket(PF_UNIX, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0), 0);
#ifdef F_GETFL
    int flags;
    ASSERT_GE(flags = fcntl(fd, F_GETFL), 0);
    ASSERT_GE(fcntl(fd, F_SETFL, flags | O_NONBLOCK), 0);
#endif
    EXPECT_EQ(android_get_control_socket(name), -1);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), ANDROID_SOCKET_DIR"/%s", name);
    unlink(addr.sun_path);

    EXPECT_EQ(bind(fd, (struct sockaddr*)&addr, sizeof(addr)), 0);
    EXPECT_EQ(android_get_control_socket(name), -1);

    char val[32];
    snprintf(val, sizeof(val), "%d", fd);
    EXPECT_EQ(setenv(key, val, true), 0);

    EXPECT_EQ(android_get_control_socket(name), fd);
    socket_close(fd);
    EXPECT_EQ(android_get_control_socket(name), -1);
    EXPECT_EQ(unlink(addr.sun_path), 0);
    EXPECT_EQ(android_get_control_socket(name), -1);
    EXPECT_EQ(unsetenv(key), 0);
    EXPECT_EQ(android_get_control_socket(name), -1);
}
