/* su.h - Store all general su info
 */

#ifndef _SU_H_
#define _SU_H_

#include <limits.h>
#include <sys/types.h>

#define SU_VERSION_STR  xstr(VERSION) ":MAGISKSU (topjohnwu)"

// Property check for root access
#define ROOT_ACCESS_PROP          "persist.sys.root_access"
#define ROOT_ACCESS_DISABLED      0
#define ROOT_ACCESS_APPS_ONLY     1
#define ROOT_ACCESS_ADB_ONLY      2
#define ROOT_ACCESS_APPS_AND_ADB  3

// DO NOT CHANGE LINE BELOW, java package name will always be the same
#define JAVA_PACKAGE_NAME "com.topjohnwu.magisk"

#define APPLICATION_DATA_PATH "/data/data/"
#define USER_DATA_PATH "/data/user"

// If --rename-manifest-package is used in AAPT, this
// must be changed to correspond to the new APK package name
// See the two Android.mk files for more details.
#define REQUESTOR JAVA_PACKAGE_NAME
// This is used if wrapping the fragment classes and activities
// with classes in another package.
#define REQUESTOR_PREFIX JAVA_PACKAGE_NAME ".superuser"
#define REQUESTOR_FILES_PATH APPLICATION_DATA_PATH REQUESTOR "/files"
#define REQUESTOR_CACHE_PATH "/dev/" REQUESTOR

// there's no guarantee that the db or files are actually created named as such by
// SQLiteOpenHelper, etc. Though that is the behavior as of current.
// it is up to the Android application to symlink as appropriate.
#define REQUESTOR_DATABASE_PATH REQUESTOR "/databases/su.db"
#define REQUESTOR_MULTIUSER_MODE REQUESTOR_FILES_PATH "/multiuser_mode"

#define DEFAULT_SHELL "/system/bin/sh"

struct su_initiator {
	pid_t pid;
	unsigned uid;
};

struct su_request {
	unsigned uid;
	int login;
	int keepenv;
	char *shell;
	char *command;
	char **argv;
	int argc;
};

struct su_user_info {
	// the user in android userspace (multiuser)
	// that invoked this action.
	unsigned android_user_id;
	// how su behaves with multiuser. see enum below.
	int multiuser_mode;
	// path to superuser directory. this is populated according
	// to the multiuser mode.
	// this is used to check uid/gid for protecting socket.
	// this is used instead of database, as it is more likely
	// to exist. db will not exist if su has never launched.
	char base_path[PATH_MAX];
	// path to su database. this is populated according
	// to the multiuser mode.
	char database_path[PATH_MAX];
};

struct su_context {
	struct su_initiator from;
	struct su_request to;
	struct su_user_info user;
	mode_t umask;
	char sock_path[PATH_MAX];
};

// multiuser su behavior
typedef enum {
	// only owner can su
	MULTIUSER_MODE_OWNER_ONLY = 0,
	// owner gets a su prompt
	MULTIUSER_MODE_OWNER_MANAGED = 1,
	// user gets a su prompt
	MULTIUSER_MODE_USER = 2,
	MULTIUSER_MODE_NONE = 3,
} multiuser_mode_t;

#define MULTIUSER_VALUE_OWNER_ONLY    "owner"
#define MULTIUSER_VALUE_OWNER_MANAGED "managed"
#define MULTIUSER_VALUE_USER          "user"
#define MULTIUSER_VALUE_NONE          "none"

typedef enum {
	INTERACTIVE = 0,
	DENY = 1,
	ALLOW = 2,
} policy_t;

extern int from_uid, from_pid;
extern int quit_signals[];

// su.c

int su_daemon_main(int argc, char **argv);

// su_client.c

int socket_create_temp(char *path, size_t len);
int socket_accept(int serv_fd);
void socket_send_request(int fd, const struct su_context *ctx);
void socket_receive_result(int fd, char *result, ssize_t result_len);

// activity.c

void app_send_result(struct su_context *ctx, policy_t policy);
void app_send_request(struct su_context *ctx);

// db.c

policy_t database_check(struct su_context *ctx);

// misc.c

void setup_sighandlers(void (*handler)(int));
void set_identity(unsigned uid);
char *get_command(const struct su_request *to);
int fork_zero_fucks();

#endif
