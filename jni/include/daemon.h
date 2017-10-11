/* daemon.h - Utility functions for daemon-client communication
 */

#ifndef _DAEMON_H_
#define _DAEMON_H_

#include <pthread.h>

extern pthread_t sepol_patch;
extern int is_restart;

// Commands require connecting to daemon
typedef enum {
	DO_NOTHING = 0,
	LAUNCH_MAGISKHIDE,
	STOP_MAGISKHIDE,
	ADD_HIDELIST,
	RM_HIDELIST,
	LS_HIDELIST,
	SUPERUSER,
	CHECK_VERSION,
	CHECK_VERSION_CODE,
	POST_FS,
	POST_FS_DATA,
	LATE_START,
	TEST
} client_request;

// Return codes for daemon
typedef enum {
	DAEMON_ERROR = -1,
	DAEMON_SUCCESS = 0,
	ROOT_REQUIRED,
	HIDE_IS_ENABLED,
	HIDE_NOT_ENABLED,
	HIDE_ITEM_EXIST,
	HIDE_ITEM_NOT_EXIST,
} daemon_response;

// daemon.c

void start_daemon();
int connect_daemon();
void auto_start_magiskhide();

// socket_trans.c

int recv_fd(int sockfd);
void send_fd(int sockfd, int fd);
int read_int(int fd);
void write_int(int fd, int val);
char* read_string(int fd);
void write_string(int fd, const char* val);

/***************
 * Boot Stages *
 ***************/

void post_fs(int client);
void post_fs_data(int client);
void late_start(int client);
void fix_filecon();

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

void su_daemon_receiver(int client);

#endif
