/* daemon.h - Utility functions for daemon-client communication
 */

// socket_trans.c
int recv_fd(int sockfd);
void send_fd(int sockfd, int fd);
int read_int(int fd);
void write_int(int fd, int val);
char* read_string(int fd);
void write_string(int fd, const char* val);
