#pragma once

#include <sys/un.h>
#include <sys/socket.h>
#include <string_view>
#include <string>
#include <vector>

struct sock_cred : public ucred {
    std::string context;
};

socklen_t setup_sockaddr(sockaddr_un *sun, const char *name);
bool get_client_cred(int fd, sock_cred *cred);
std::vector<int> recv_fds(int sockfd);
int recv_fd(int sockfd);
int send_fds(int sockfd, const int *fds, int cnt);
int send_fd(int sockfd, int fd);
int read_int(int fd);
int read_int_be(int fd);
void write_int(int fd, int val);
void write_int_be(int fd, int val);
std::string read_string(int fd);
bool read_string(int fd, std::string &str);
void write_string(int fd, std::string_view str);
