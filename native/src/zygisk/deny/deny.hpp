#pragma once

#include <pthread.h>
#include <string_view>
#include <functional>
#include <map>
#include <atomic>

#include <daemon.hpp>

#define ISOLATED_MAGIC "isolated"

namespace DenyRequest {
enum : int {
    ENFORCE,
    DISABLE,
    ADD,
    REMOVE,
    LIST,
    STATUS,

    END
};
}

namespace DenyResponse {
enum : int {
    OK,
    ENFORCED,
    NOT_ENFORCED,
    ITEM_EXIST,
    ITEM_NOT_EXIST,
    INVALID_PKG,
    NO_NS,
    ERROR,

    END
};
}

// CLI entries
int enable_deny();
int disable_deny();
int add_list(int client);
int rm_list(int client);
void ls_list(int client);

// Utility functions
bool is_deny_target(int uid, std::string_view process);

// Revert
void revert_daemon(int pid, int client);
void revert_unmount(int pid = -1, const char *ref_pid = "self");

extern int sys_ui_app_id;
extern std::atomic<bool> denylist_enforced;
