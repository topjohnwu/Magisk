#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <memory>

#include <db.hpp>

#define DEFAULT_SHELL "/system/bin/sh"

// Constants for atty
#define ATTY_IN    (1 << 0)
#define ATTY_OUT   (1 << 1)
#define ATTY_ERR   (1 << 2)

class su_info {
public:
    /* Unique key */
    const int uid;

    /* These should be guarded with internal lock */
    db_settings cfg;
    db_strings str;
    su_access access;
    struct stat mgr_st;

    /* This should be guarded with global cache lock */
    long timestamp;

    su_info(unsigned uid = 0);
    ~su_info();
    mutex_guard lock();
    bool is_fresh();
    void refresh();

private:
    pthread_mutex_t _lock;  /* Internal lock */
};

struct su_req_base {
    int uid = UID_ROOT;
    bool login = false;
    bool keepenv = false;
    bool mount_master = false;
} __attribute__((packed));

struct su_request : public su_req_base {
    std::string shell = DEFAULT_SHELL;
    std::string command;
};

struct su_context {
    std::shared_ptr<su_info> info;
    su_request req;
    int pid;
};

void app_log(const su_context &ctx);
void app_notify(const su_context &ctx);
int app_request(const std::shared_ptr<su_info> &info);
