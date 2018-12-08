/* daemon.h - Utility functions for daemon-client communication
 */

#ifndef _DAEMON_H_
#define _DAEMON_H_

#include <stdbool.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>

// Commands require connecting to daemon
enum {
	DO_NOTHING = 0,
	SUPERUSER,
	CHECK_VERSION,
	CHECK_VERSION_CODE,
	POST_FS_DATA,
	LATE_START,
	BOOT_COMPLETE,
	MAGISKHIDE,
	HIDE_CONNECT,
	HANDSHAKE,
	SQLITE_CMD,
};

// Return codes for daemon
enum {
	DAEMON_ERROR = -1,
	DAEMON_SUCCESS = 0,
	ROOT_REQUIRED,
	DAEMON_LAST
};

// daemon.c

int connect_daemon();
int switch_mnt_ns(int pid);

// log_monitor.c

extern bool log_daemon_started;
int connect_log_daemon();
bool start_log_daemon();

// socket.c

socklen_t setup_sockaddr(struct sockaddr_un *sun, const char *name);
int create_rand_socket(struct sockaddr_un *sun);
int socket_accept(int sockfd, int timeout);
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

/***************
 * Boot Stages *
 ***************/

void unlock_blocks();
void startup();
void post_fs_data(int client);
void late_start(int client);
void boot_complete(int client);

/**************
 * MagiskHide *
 **************/

void magiskhide_handler(int client);

/*************
 * Superuser *
 *************/

void su_daemon_handler(int client, struct ucred *credential);

#endif
