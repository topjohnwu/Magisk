/* daemon.h - Utility functions for daemon-client communication
 */

#ifndef _DAEMON_H_
#define _DAEMON_H_

#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>

extern int is_daemon_init;
extern int seperate_vendor;
extern int full_patch_pid;

// Commands require connecting to daemon
enum {
	DO_NOTHING = 0,
	SUPERUSER,
	CHECK_VERSION,
	CHECK_VERSION_CODE,
	POST_FS_DATA,
	LATE_START,
	LAUNCH_MAGISKHIDE,
	STOP_MAGISKHIDE,
	ADD_HIDELIST,
	RM_HIDELIST,
	LS_HIDELIST
};

// Return codes for daemon
enum {
	DAEMON_ERROR = -1,
	DAEMON_SUCCESS = 0,
	ROOT_REQUIRED,
	LOGCAT_DISABLED,
	HIDE_IS_ENABLED,
	HIDE_NOT_ENABLED,
	HIDE_ITEM_EXIST,
	HIDE_ITEM_NOT_EXIST,
};

// daemon.c

void start_daemon(int post_fs_data);
int connect_daemon(int post_fs_data);
void auto_start_magiskhide();

// socket.c

int setup_socket(struct sockaddr_un *sun);
int recv_fd(int sockfd);
void send_fd(int sockfd, int fd);
int read_int(int fd);
void write_int(int fd, int val);
char* read_string(int fd);
void write_string(int fd, const char* val);

/***************
 * Boot Stages *
 ***************/

void startup();
void post_fs_data(int client);
void late_start(int client);

/**************
 * MagiskHide *
 **************/

void launch_magiskhide(int client);
void stop_magiskhide(int client);
void add_hide_list(int client);
void rm_hide_list(int client);
void ls_hide_list(int client);

/*************
 * Superuser *
 *************/

void su_daemon_receiver(int client, struct ucred *credential);

#endif
