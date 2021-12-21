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

extern bool initialize_windows_sockets();

SOCKET socket_network_client(const char* host, int port, int type) {
    if (!initialize_windows_sockets()) {
        return INVALID_SOCKET;
    }

    // First resolve the host and port parameters into a usable network address.
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = type;

    struct addrinfo* address = NULL;
    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", port);
    if (getaddrinfo(host, port_str, &hints, &address) != 0 || address == NULL) {
        if (address != NULL) {
            freeaddrinfo(address);
        }
        return INVALID_SOCKET;
    }

    // Now create and connect the socket.
    SOCKET sock = socket(address->ai_family, address->ai_socktype,
                         address->ai_protocol);
    if (sock == INVALID_SOCKET) {
        freeaddrinfo(address);
        return INVALID_SOCKET;
    }

    if (connect(sock, address->ai_addr, address->ai_addrlen) == SOCKET_ERROR) {
        closesocket(sock);
        freeaddrinfo(address);
        return INVALID_SOCKET;
    }

    freeaddrinfo(address);
    return sock;
}
