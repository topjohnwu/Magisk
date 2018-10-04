/* su.h - Store all general su info
 */

#ifndef _SU_H_
#define _SU_H_

#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "db.h"

#define DEFAULT_SHELL "/system/bin/sh"

// Constants for atty
#define ATTY_IN     1
#define ATTY_OUT    2
#define ATTY_ERR    4

struct su_info {
	unsigned uid;          /* Unique key to find su_info */
	pthread_mutex_t lock;  /* Internal lock */
	int count;             /* Just a count for debugging purpose */

	/* These values should be guarded with internal lock */
	struct db_settings dbs;
	struct db_strings str;
	struct su_access access;
	struct stat mgr_st;

	/* These should be guarded with global cache lock */
	int ref;
	int life;
};

#define DB_SET(i, e) (i)->dbs.v[e]
#define DB_STR(i, e) (i)->str.s[e]

struct su_request {
	unsigned uid;
	unsigned login;
	unsigned keepenv;
	char *shell;
	char *command;
};

struct su_context {
	struct su_info *info;
	struct su_request req;
	pid_t pid;
};

// connect.c

void app_log(struct su_context *ctx);
void app_connect(const char *socket, struct su_info *info);
void socket_send_request(int fd, struct su_info *info);

#endif
