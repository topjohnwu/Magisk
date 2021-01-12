#pragma once

#include <sys/un.h>
#include <sys/socket.h>
#include <string_view>

socklen_t setup_sockaddr(sockaddr_un *sun, const char *name);
int socket_accept(int sockfd, int timeout);
void get_client_cred(int fd, struct ucred *cred);
int recv_fd(int sockfd);
void send_fd(int sockfd, int fd);
int read_int(int fd);
int read_int_be(int fd);
void write_int(int fd, int val);
void write_int_be(int fd, int val);
std::string read_string(int fd);
void read_string(int fd, std::string &str);
void write_string(int fd, std::string_view str);
