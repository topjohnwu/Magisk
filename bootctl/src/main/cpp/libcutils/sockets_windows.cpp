/*
 * Copyright (C) 2015 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <cutils/sockets.h>

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms741549(v=vs.85).aspx
// claims WSACleanup() should be called before program exit, but general
// consensus seems to be that it hasn't actually been necessary for a long time,
// likely since Windows 3.1. Additionally, trying to properly use WSACleanup()
// can be extremely tricky and cause deadlock when using threads or atexit().
//
// Both adb (1) and Chrome (2) purposefully avoid WSACleanup() with no issues.
// (1) https://android.googlesource.com/platform/system/core.git/+/master/adb/sysdeps_win32.cpp
// (2) https://code.google.com/p/chromium/codesearch#chromium/src/net/base/winsock_init.cc
bool initialize_windows_sockets() {
    // There's no harm in calling WSAStartup() multiple times but no benefit
    // either, we may as well skip it after the first.
    static bool init_success = false;

    if (!init_success) {
        WSADATA wsaData;
        init_success = (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);
    }

    return init_success;
}

int socket_close(cutils_socket_t sock) {
    return closesocket(sock);
}

ssize_t socket_send_buffers(cutils_socket_t sock,
                            const cutils_socket_buffer_t* buffers,
                            size_t num_buffers) {
    if (num_buffers > SOCKET_SEND_BUFFERS_MAX_BUFFERS) {
        return -1;
    }

    WSABUF wsa_buffers[SOCKET_SEND_BUFFERS_MAX_BUFFERS];
    for (size_t i = 0; i < num_buffers; ++i) {
        // It's safe to cast away const here; WSABUF declares non-const
        // void* because it's used for both send and receive, but since
        // we're only sending, the data won't be modified.
        wsa_buffers[i].buf =
                reinterpret_cast<char*>(const_cast<void*>(buffers[i].data));
        wsa_buffers[i].len = buffers[i].length;
    }

    DWORD bytes_sent = 0;
    if (WSASend(sock, wsa_buffers, num_buffers, &bytes_sent, 0, nullptr,
                nullptr) != SOCKET_ERROR) {
        return bytes_sent;
    }

    return -1;
}

int android_get_control_socket(const char*) {
    return -1;
}
