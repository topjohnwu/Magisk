/* su.h - Store all general su info
 */

#ifndef _SU_H_
#define _SU_H_

#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "db.h"
#include "list.h"

#define MAGISKSU_VER_STR  xstr(MAGISK_VERSION) ":MAGISKSU (topjohnwu)"

// This is used if wrapping the fragment classes and activities
// with classes in another package.
#define REQUESTOR_PREFIX JAVA_PACKAGE_NAME ".superuser"

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
	char sock_path[PATH_MAX];
	int pipefd[2];
};

extern struct su_context *su_ctx;

// su.c

int su_daemon_main(int argc, char **argv);
__attribute__ ((noreturn)) void exit2(int status);
void set_identity(unsigned uid);

// su_client.c

int socket_create_temp(char *path, size_t len);
int socket_accept(int serv_fd);
void socket_send_request(int fd, const struct su_context *ctx);
void socket_receive_result(int fd, char *result, ssize_t result_len);

// activity.c

void app_send_result(struct su_context *ctx, policy_t policy);
void app_send_request(struct su_context *ctx);

#endif
