/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include <sys/un.h>
#include <unistd.h>

#include <android-base/file.h>
#include <android-base/stringprintf.h>
#include <android-base/unique_fd.h>
#include <gtest/gtest.h>

using android::base::StringPrintf;
using android::base::unique_fd;

// logd_writer takes advantage of the fact that connect() can be called multiple times for a DGRAM
// socket.  This tests for that behavior.
TEST(liblog, multi_connect_dgram_socket) {
#ifdef __ANDROID__
  if (getuid() != 0) {
    GTEST_SKIP() << "Skipping test, must be run as root.";
    return;
  }
  auto temp_dir = TemporaryDir();
  auto socket_path = StringPrintf("%s/test_socket", temp_dir.path);

  unique_fd server_socket;

  auto open_server_socket = [&] {
    server_socket.reset(TEMP_FAILURE_RETRY(socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0)));
    ASSERT_TRUE(server_socket.ok());

    sockaddr_un server_sockaddr = {};
    server_sockaddr.sun_family = AF_UNIX;
    strlcpy(server_sockaddr.sun_path, socket_path.c_str(), sizeof(server_sockaddr.sun_path));
    ASSERT_EQ(0,
              TEMP_FAILURE_RETRY(bind(server_socket, reinterpret_cast<sockaddr*>(&server_sockaddr),
                                      sizeof(server_sockaddr))));
  };

  // Open the server socket.
  open_server_socket();

  // Open the client socket.
  auto client_socket =
      unique_fd{TEMP_FAILURE_RETRY(socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0))};
  ASSERT_TRUE(client_socket.ok());
  sockaddr_un client_sockaddr = {};
  client_sockaddr.sun_family = AF_UNIX;
  strlcpy(client_sockaddr.sun_path, socket_path.c_str(), sizeof(client_sockaddr.sun_path));
  ASSERT_EQ(0,
            TEMP_FAILURE_RETRY(connect(client_socket, reinterpret_cast<sockaddr*>(&client_sockaddr),
                                       sizeof(client_sockaddr))));

  // Ensure that communication works.
  constexpr static char kSmoke[] = "smoke test";
  ssize_t smoke_len = sizeof(kSmoke);
  ASSERT_EQ(smoke_len, TEMP_FAILURE_RETRY(write(client_socket, kSmoke, sizeof(kSmoke))));
  char read_buf[512];
  ASSERT_EQ(smoke_len, TEMP_FAILURE_RETRY(read(server_socket, read_buf, sizeof(read_buf))));
  ASSERT_STREQ(kSmoke, read_buf);

  // Close the server socket.
  server_socket.reset();
  ASSERT_EQ(0, unlink(socket_path.c_str())) << strerror(errno);

  // Ensure that write() from the client returns an error since the server is closed.
  ASSERT_EQ(-1, TEMP_FAILURE_RETRY(write(client_socket, kSmoke, sizeof(kSmoke))));
  ASSERT_EQ(errno, ECONNREFUSED) << strerror(errno);

  // Open the server socket again.
  open_server_socket();

  // Reconnect the same client socket.
  ASSERT_EQ(0,
            TEMP_FAILURE_RETRY(connect(client_socket, reinterpret_cast<sockaddr*>(&client_sockaddr),
                                       sizeof(client_sockaddr))))
      << strerror(errno);

  // Ensure that communication works.
  ASSERT_EQ(smoke_len, TEMP_FAILURE_RETRY(write(client_socket, kSmoke, sizeof(kSmoke))));
  ASSERT_EQ(smoke_len, TEMP_FAILURE_RETRY(read(server_socket, read_buf, sizeof(read_buf))));
  ASSERT_STREQ(kSmoke, read_buf);
#else
  GTEST_LOG_(INFO) << "This test does nothing.\n";
#endif
}