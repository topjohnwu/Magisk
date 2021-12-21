/*
 * Copyright (C) 2015 The Android Open Source Project
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

// Tests socket functionality using loopback connections. Requires IPv4 and
// IPv6 capabilities. These tests assume that no UDP packets are lost, which
// should be the case for loopback communication, but is not guaranteed.

#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

#include <cutils/sockets.h>
#include <gtest/gtest.h>

// Makes sure the passed sockets are valid, sends data between them, and closes
// them. Any failures are logged with gtest.
//
// On Mac recvfrom() will not fill in the address for TCP sockets, so we need
// separate logic paths depending on socket type.
static void TestConnectedSockets(cutils_socket_t server, cutils_socket_t client,
                                 int type) {
    ASSERT_NE(INVALID_SOCKET, server);
    ASSERT_NE(INVALID_SOCKET, client);

    char buffer[128];
    sockaddr_storage addr;
    socklen_t addr_size = sizeof(addr);

    // Send client -> server first to get the UDP client's address.
    ASSERT_EQ(3, send(client, "foo", 3, 0));
    if (type == SOCK_DGRAM) {
        EXPECT_EQ(3, recvfrom(server, buffer, sizeof(buffer), 0,
                              reinterpret_cast<sockaddr*>(&addr), &addr_size));
    } else {
        EXPECT_EQ(3, recv(server, buffer, sizeof(buffer), 0));
    }
    EXPECT_EQ(0, memcmp(buffer, "foo", 3));

    // Now send server -> client.
    if (type == SOCK_DGRAM) {
        ASSERT_EQ(3, sendto(server, "bar", 3, 0,
                            reinterpret_cast<sockaddr*>(&addr), addr_size));
    } else {
        ASSERT_EQ(3, send(server, "bar", 3, 0));
    }
    EXPECT_EQ(3, recv(client, buffer, sizeof(buffer), 0));
    EXPECT_EQ(0, memcmp(buffer, "bar", 3));

    // Send multiple buffers using socket_send_buffers().
    std::string data[] = {"foo", "bar", "12345"};
    cutils_socket_buffer_t socket_buffers[] = { {data[0].data(), data[0].length()},
                                                {data[1].data(), data[1].length()},
                                                {data[2].data(), data[2].length()} };
    EXPECT_EQ(11, socket_send_buffers(client, socket_buffers, 3));
    EXPECT_EQ(11, recv(server, buffer, sizeof(buffer), 0));
    EXPECT_EQ(0, memcmp(buffer, "foobar12345", 11));

    EXPECT_EQ(0, socket_close(server));
    EXPECT_EQ(0, socket_close(client));
}

// Tests socket_get_local_port().
TEST(SocketsTest, TestGetLocalPort) {
    cutils_socket_t server;

    // Check a bunch of ports so that we can ignore any conflicts in case
    // of ports already being taken, but if a server is able to start up we
    // should always be able to read its port.
    for (int port : {10000, 12345, 15999, 20202, 25000}) {
        for (int type : {SOCK_DGRAM, SOCK_STREAM}) {
            server = socket_inaddr_any_server(port, type);
            if (server != INVALID_SOCKET) {
                EXPECT_EQ(port, socket_get_local_port(server));
            }
            socket_close(server);
        }
    }

    // Check expected failure for an invalid socket.
    EXPECT_EQ(-1, socket_get_local_port(INVALID_SOCKET));
}

// Tests socket_inaddr_any_server() and socket_network_client() for IPv4 UDP.
TEST(SocketsTest, TestIpv4UdpLoopback) {
    cutils_socket_t server = socket_inaddr_any_server(0, SOCK_DGRAM);
    cutils_socket_t client = socket_network_client(
            "127.0.0.1", socket_get_local_port(server), SOCK_DGRAM);

    TestConnectedSockets(server, client, SOCK_DGRAM);
}

// Tests socket_inaddr_any_server() and socket_network_client() for IPv4 TCP.
TEST(SocketsTest, TestIpv4TcpLoopback) {
    cutils_socket_t server = socket_inaddr_any_server(0, SOCK_STREAM);
    ASSERT_NE(INVALID_SOCKET, server);

    cutils_socket_t client = socket_network_client(
            "127.0.0.1", socket_get_local_port(server), SOCK_STREAM);
    cutils_socket_t handler = accept(server, nullptr, nullptr);
    EXPECT_EQ(0, socket_close(server));

    TestConnectedSockets(handler, client, SOCK_STREAM);
}

// Tests socket_inaddr_any_server() and socket_network_client() for IPv6 UDP.
TEST(SocketsTest, TestIpv6UdpLoopback) {
    cutils_socket_t server = socket_inaddr_any_server(0, SOCK_DGRAM);
    cutils_socket_t client = socket_network_client(
            "::1", socket_get_local_port(server), SOCK_DGRAM);

    TestConnectedSockets(server, client, SOCK_DGRAM);
}

// Tests socket_inaddr_any_server() and socket_network_client() for IPv6 TCP.
TEST(SocketsTest, TestIpv6TcpLoopback) {
    cutils_socket_t server = socket_inaddr_any_server(0, SOCK_STREAM);
    ASSERT_NE(INVALID_SOCKET, server);

    cutils_socket_t client = socket_network_client(
            "::1", socket_get_local_port(server), SOCK_STREAM);
    cutils_socket_t handler = accept(server, nullptr, nullptr);
    EXPECT_EQ(0, socket_close(server));

    TestConnectedSockets(handler, client, SOCK_STREAM);
}

// Tests socket_send_buffers() failure.
TEST(SocketsTest, TestSocketSendBuffersFailure) {
    EXPECT_EQ(-1, socket_send_buffers(INVALID_SOCKET, nullptr, 0));
}
