/* su.h - Store all general su info
 */

#ifndef _SU_H_
#define _SU_H_

#include <limits.h>
#include <sys/types.h>

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

// DO NOT CHANGE LINE BELOW, java package name will always be the same
#define JAVA_PACKAGE_NAME "com.topjohnwu.magisk"

#define APP_DATA_PATH "/data/data/"
#define USER_DATA_PATH "/data/user"

// If --rename-manifest-package is used in AAPT, this
// must be changed to correspond to the new APK package name
// See the two Android.mk files for more details.
#define REQUESTOR JAVA_PACKAGE_NAME
// This is used if wrapping the fragment classes and activities
// with classes in another package.
#define REQUESTOR_PREFIX JAVA_PACKAGE_NAME ".superuser"
#define REQUESTOR_CACHE_PATH "/dev/" REQUESTOR
// there's no guarantee that the db or files are actually created named as such by
// SQLiteOpenHelper, etc. Though that is the behavior as of current.
// it is up to the Android application to symlink as appropriate.
#define REQUESTOR_DATABASE_PATH REQUESTOR "/databases/su.db"

#define DEFAULT_SHELL "/system/bin/sh"

typedef enum {
    QUERY = 0,
    DENY = 1,
    ALLOW = 2,
} policy_t;

struct su_info {
    unsigned uid;
    policy_t policy;
    pthread_mutex_t lock;
    int count;
    int clock;
    int multiuser_mode;
    int root_access;
    int mnt_ns;
    struct list_head pos;
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
    struct su_info *info;
    struct su_request to;
    struct su_user_info user;
    pid_t pid;
    int notify;
    mode_t umask;
    char sock_path[PATH_MAX];
};

extern struct su_context *su_ctx;
extern int pipefd[2];

// su.c

int su_daemon_main(int argc, char **argv);
__attribute__ ((noreturn)) void exit2(int status);

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

// misc.c

void set_identity(unsigned uid);
char *get_command(const struct su_request *to);
int fork_zero_fucks();

#endif
