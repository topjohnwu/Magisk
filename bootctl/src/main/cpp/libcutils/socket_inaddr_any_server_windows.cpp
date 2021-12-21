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

#include <errno.h>

#define LISTEN_BACKLOG 4

extern bool initialize_windows_sockets();

SOCKET socket_inaddr_any_server(int port, int type) {
    if (!initialize_windows_sockets()) {
      return INVALID_SOCKET;
    }

    SOCKET sock = socket(AF_INET6, type, 0);
    if (sock == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }

    // Enforce exclusive addresses so nobody can steal the port from us (1),
    // and enable dual-stack so both IPv4 and IPv6 work (2).
    // (1) https://msdn.microsoft.com/en-us/library/windows/desktop/ms740621(v=vs.85).aspx.
    // (2) https://msdn.microsoft.com/en-us/library/windows/desktop/bb513665(v=vs.85).aspx.
    int exclusive = 1;
    DWORD v6_only = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&exclusive,
                   sizeof(exclusive)) == SOCKET_ERROR ||
        setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&v6_only,
                   sizeof(v6_only)) == SOCKET_ERROR) {
        closesocket(sock);
        return INVALID_SOCKET;
    }

    // Bind the socket to our local port.
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port);
    addr.sin6_addr = in6addr_any;
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(sock);
        return INVALID_SOCKET;
    }

    // Start listening for connections if this is a TCP socket.
    if (type == SOCK_STREAM && listen(sock, LISTEN_BACKLOG) == SOCKET_ERROR) {
        closesocket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}
