#pragma once

#include <sys/un.h>
#include <sys/socket.h>

socklen_t setup_sockaddr(struct sockaddr_un *sun, const char *name);
int create_rand_socket(struct sockaddr_un *sun);
int socket_accept(int sockfd, int timeout);
void get_client_cred(int fd, struct ucred *cred);
int recv_fd(int sockfd);
void send_fd(int sockfd, int fd);
int read_int(int fd);
int read_int_be(int fd);
void write_int(int fd, int val);
void write_int_be(int fd, int val);
char *read_string(int fd);
char *read_string_be(int fd);
void write_string(int fd, const char *val);
void write_string_be(int fd, const char *val);
void write_key_value(int fd, const char *key, const char *val);
void write_key_token(int fd, const char *key, int tok);
