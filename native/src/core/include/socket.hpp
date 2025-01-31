#pragma once

#include <sys/un.h>
#include <sys/socket.h>
#include <string_view>
#include <string>
#include <vector>

#include <base.hpp>

struct sock_cred : public ucred {
    std::string context;
};

template<typename T> requires(std::is_trivially_copyable_v<T>)
T read_any(int fd) {
    T val;
    if (xxread(fd, &val, sizeof(val)) != sizeof(val))
        return -1;
    return val;
}

template<typename T> requires(std::is_trivially_copyable_v<T>)
void write_any(int fd, T val) {
    if (fd < 0) return;
    xwrite(fd, &val, sizeof(val));
}

template<typename T> requires(std::is_trivially_copyable_v<T>)
void write_vector(int fd, const std::vector<T> &vec) {
    write_any(fd, vec.size());
    xwrite(fd, vec.data(), vec.size() * sizeof(T));
}

template<typename T> requires(std::is_trivially_copyable_v<T>)
bool read_vector(int fd, std::vector<T> &vec) {
    auto size = read_any<size_t>(fd);
    vec.resize(size);
    return xread(fd, vec.data(), size * sizeof(T)) == size * sizeof(T);
}

bool get_client_cred(int fd, sock_cred *cred);
static inline int read_int(int fd) { return read_any<int>(fd); }
int read_int_be(int fd);
static inline void write_int(int fd, int val) { write_any(fd, val); }
void write_int_be(int fd, int val);
std::string read_string(int fd);
bool read_string(int fd, std::string &str);
void write_string(int fd, std::string_view str);
