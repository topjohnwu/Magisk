#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <memory>

#include <sqlite.hpp>
#include <core.hpp>

#define DEFAULT_SHELL "/system/bin/sh"

// Constants for atty
#define ATTY_IN    (1 << 0)
#define ATTY_OUT   (1 << 1)
#define ATTY_ERR   (1 << 2)

typedef enum {
    QUERY = 0,
    DENY = 1,
    ALLOW = 2,
} policy_t;

struct su_access {
    policy_t policy;
    int log;
    int notify;

    su_access() : policy(QUERY), log(1), notify(1) {}

    void operator()(StringSlice columns, const DbValues &data);
    void silent_deny() {
        policy = DENY;
        log = 0;
        notify = 0;
    }
    void silent_allow() {
        policy = ALLOW;
        log = 0;
        notify = 0;
    }
};

class su_info {
public:
    // Unique key
    const int uid;

    // These should be guarded with internal lock
    int eval_uid;  // The effective UID, taking multiuser settings into consideration
    struct DbSettings cfg;
    su_access access;
    std::string mgr_pkg;
    int mgr_uid;
    void check_db();

    // These should be guarded with global cache lock
    bool is_fresh();
    void refresh();

    su_info(int uid);
    ~su_info();
    mutex_guard lock();

private:
    long timestamp;
    // Internal lock
    pthread_mutex_t _lock;
};

struct su_req_base {
    uid_t uid = AID_ROOT;
    bool login = false;
    bool keepenv = false;
    pid_t target = -1;
} __attribute__((packed));

struct su_request : public su_req_base {
    std::string shell = DEFAULT_SHELL;
    std::string command;
    std::string context;
    std::vector<gid_t> gids;
};

struct su_context {
    std::shared_ptr<su_info> info;
    su_request req;
    int pid;
};

void app_log(const su_context &ctx);
void app_notify(const su_context &ctx);
int app_request(const su_context &ctx);
