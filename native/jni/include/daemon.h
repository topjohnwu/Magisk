/* daemon.h - Utility functions for daemon-client communication
 */

#ifndef _DAEMON_H_
#define _DAEMON_H_

#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <string>
#include <vector>

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
	SQLITE_CMD,
	ZYGOTE_NOTIFY,
};

// Return codes for daemon
enum {
	DAEMON_ERROR = -1,
	DAEMON_SUCCESS = 0,
	ROOT_REQUIRED,
	DAEMON_LAST
};

// daemon.c

int connect_daemon(bool create = false);
int switch_mnt_ns(int pid);

// socket.c

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

/***************
 * Boot Stages *
 ***************/

void unlock_blocks();
void post_fs_data(int client);
void late_start(int client);
void boot_complete(int client);

/*************
 * Scripting *
 *************/

void exec_script(const char *script);
void exec_common_script(const char *stage);
void exec_module_script(const char *stage, const std::vector<std::string> &module_list);
void migrate_img(const char *img);
void install_apk(const char *apk);

/**************
 * MagiskHide *
 **************/

void magiskhide_handler(int client);

/*************
 * Superuser *
 *************/

void su_daemon_handler(int client, struct ucred *credential);

#endif
