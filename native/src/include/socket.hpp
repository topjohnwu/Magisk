#pragma once

#include <sys/un.h>
#include <sys/socket.h>
#include <string_view>
#include <string>
#include <vector>

struct sock_cred : public ucred {
    std::string context;
};

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

template<typename T> requires(std::is_trivially_copyable_v<T>)
void write_vector(int fd, const std::vector<T> &vec) {
    write_int(fd, static_cast<int>(vec.size()));
    xwrite(fd, vec.data(), vec.size() * sizeof(T));
}

template<typename T> requires(std::is_trivially_copyable_v<T>)
bool read_vector(int fd, std::vector<T> &vec) {
    int size = read_int(fd);
    if (size == -1) return false;
    vec.resize(size);
    return xread(fd, vec.data(), size * sizeof(T)) == size * sizeof(T);
}
