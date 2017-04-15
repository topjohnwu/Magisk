/* daemon.h - Utility functions for daemon-client communication
 */

#ifndef _DAEMON_H_
#define _DAEMON_H_

#include <pthread.h>

extern pthread_t sepol_patch;

// Commands require connecting to daemon
typedef enum {
	LAUNCH_MAGISKHIDE,
	STOP_MAGISKHIDE,
	ADD_HIDELIST,
	RM_HIDELIST,
	SUPERUSER,
	CHECK_VERSION,
	CHECK_VERSION_CODE,
	POST_FS,
	POST_FS_DATA,
	LATE_START,
	TEST
} client_request;

// daemon.c

void start_daemon();
int connect_daemon();

// socket_trans.c

int recv_fd(int sockfd);
void send_fd(int sockfd, int fd);
int read_int(int fd);
void write_int(int fd, int val);
char* read_string(int fd);
void write_string(int fd, const char* val);

// log_monitor.c

void monitor_logs();

/***************
 * Boot Stages *
 ***************/

void post_fs(int client);
void post_fs_data(int client);
void late_start(int client);

/**************
 * MagiskHide *
 **************/

void launch_magiskhide(int client);
void stop_magiskhide(int client);

/*************
 * Superuser *
 *************/

void su_daemon_receiver(int client);

#endif
