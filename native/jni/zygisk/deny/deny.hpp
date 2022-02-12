#pragma once

#include <pthread.h>
#include <string_view>
#include <functional>
#include <map>
#include <atomic>

#include <daemon.hpp>

#define ISOLATED_MAGIC "isolated"

enum class DenyRequest : int {
    ENFORCE,
    DISABLE,
    ADD,
    REMOVE,
    LIST,
    STATUS,

    END
};

enum class DenyResponse: int {
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


// CLI entries
DenyResponse enable_deny();
DenyResponse disable_deny();
DenyResponse add_list(int client);
DenyResponse rm_list(int client);
void ls_list(int client);

// Utility functions
bool is_deny_target(int uid, std::string_view process);

void revert_unmount();

extern std::atomic<bool> denylist_enforced;
extern std::atomic<int> cached_manager_app_id;

inline int deny_request(DenyRequest req) {
    int fd = connect_daemon(DaemonRequest::DENYLIST);
    write_int(fd, static_cast<std::underlying_type_t<DenyRequest>>(req));
    return fd;
}
