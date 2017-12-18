/* su.h - Store all general su info
 */

#ifndef _SU_H_
#define _SU_H_

#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "list.h"

#define MAGISKSU_VER_STR  xstr(MAGISK_VERSION) ":MAGISKSU (topjohnwu)"

// DB settings for root access
#define ROOT_ACCESS_ENTRY         "root_access"
#define ROOT_ACCESS_DISABLED      0
#define ROOT_ACCESS_APPS_ONLY     1
#define ROOT_ACCESS_ADB_ONLY      2
#define ROOT_ACCESS_APPS_AND_ADB  3

// DB settings for multiuser
#define MULTIUSER_MODE_ENTRY            "multiuser_mode"
#define MULTIUSER_MODE_OWNER_ONLY       0
#define MULTIUSER_MODE_OWNER_MANAGED    1
#define MULTIUSER_MODE_USER             2

// DB settings for namespace seperation
#define NAMESPACE_MODE_ENTRY      "mnt_ns"
#define NAMESPACE_MODE_GLOBAL     0
#define NAMESPACE_MODE_REQUESTER  1
#define NAMESPACE_MODE_ISOLATE    2

// DB entry for requester
#define REQUESTER_ENTRY           "requester"

// DO NOT CHANGE LINE BELOW, java package name will always be the same
#define JAVA_PACKAGE_NAME "com.topjohnwu.magisk"
// This is used if wrapping the fragment classes and activities
// with classes in another package.
#define REQUESTOR_PREFIX JAVA_PACKAGE_NAME ".superuser"

#define DEFAULT_SHELL "/system/bin/sh"
#define DATABASE_PATH "/data/adb/magisk.db"

typedef enum {
	QUERY = 0,
	DENY = 1,
	ALLOW = 2,
} policy_t;

struct su_info {
	unsigned uid;  /* Key to find su_info */
	pthread_mutex_t lock;  /* Internal lock */
	int count;  /* Just a count for debugging purpose */

	/* These values should be guarded with internal lock */
	policy_t policy;
	int multiuser_mode;
	int root_access;
	int mnt_ns;
	char pkg_name[PATH_MAX];
	struct stat st;

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
	int notify;
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

// db.c

void database_check(struct su_context *ctx);

#endif
