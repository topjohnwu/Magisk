/* su.h - Store all general su info
 */

#ifndef _SU_H_
#define _SU_H_

#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "db.h"
#include "list.h"

#define DEFAULT_SHELL "/system/bin/sh"

struct su_info {
	unsigned uid;          /* Unique key to find su_info */
	pthread_mutex_t lock;  /* Internal lock */
	int count;             /* Just a count for debugging purpose */

	/* These values should be guarded with internal lock */
	struct db_settings dbs;
	struct db_strings str;
	struct su_access access;
	struct stat manager_stat;

	/* These should be guarded with global list lock */
	struct list_head pos;
	int ref;
	int clock;
};

struct su_request {
	unsigned uid;
	int login;
	int keepenv;
	char *shell;
	char *command;
};

struct su_context {
	struct su_info *info;
	struct su_request to;
	pid_t pid;
	char cwd[PATH_MAX];
	int pipefd[2];
};

extern struct su_context *su_ctx;

// su.c

int su_daemon_main(int argc, char **argv);
__attribute__ ((noreturn)) void exit2(int status);
void set_identity(unsigned uid);

// connect.c

void app_log();

void app_connect(const char *socket);
void socket_send_request(int fd);

#endif
