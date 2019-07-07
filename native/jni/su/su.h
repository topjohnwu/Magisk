#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <memory>

#include <db.h>

#define DEFAULT_SHELL "/system/bin/sh"

// Constants for atty
#define ATTY_IN    (1 << 0)
#define ATTY_OUT   (1 << 1)
#define ATTY_ERR   (1 << 2)

class su_info {
public:
	/* Unique key */
	const unsigned uid;

	/* These should be guarded with internal lock */
	db_settings cfg;
	db_strings str;
	su_access access;
	struct stat mgr_st;

	/* This should be guarded with global cache lock */
	long timestamp;

	su_info(unsigned uid = 0);
	~su_info();
	void lock();
	void unlock();
	bool is_fresh();
	void refresh();

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
	std::shared_ptr<su_info> info;
	su_request req;
	pid_t pid;
};

// connect.c

void app_log(const su_context &ctx);
void app_notify(const su_context &ctx);
void app_connect(const char *socket, const std::shared_ptr<su_info> &info);
void socket_send_request(int fd, const std::shared_ptr<su_info> &info);
