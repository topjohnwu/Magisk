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

class su_info {
public:
	unsigned uid;          /* Unique key to find su_info */
	int count;             /* Just a count for debugging purpose */

	/* These values should be guarded with internal lock */
	db_settings cfg;
	db_strings str;
	su_access access;
	struct stat mgr_st;

	/* These should be guarded with global cache lock */
	int ref;
	time_t timestamp;

	su_info(unsigned uid);
	~su_info();
	void lock();
	void unlock();
	bool isFresh();
	void newRef();
	void deRef();

private:
	pthread_mutex_t _lock;  /* Internal lock */
};

struct su_req_base {
	unsigned uid;
	bool login;
	bool keepenv;
	bool mount_master;
protected:
	su_req_base();
} __attribute__((packed));

struct su_request : public su_req_base {
	const char *shell;
	const char *command;
	su_request();
} __attribute__((packed));

struct su_context {
	struct su_info *info;
	struct su_request req;
	pid_t pid;
};

// connect.c

void app_log(struct su_context *ctx);
void app_notify(struct su_context *ctx);
void app_connect(const char *socket, struct su_info *info);
void socket_send_request(int fd, struct su_info *info);

#endif
