#pragma once

#include <string_view>

#define ISOLATED_MAGIC "isolated"
#define WEBVIEW_ZYGOTE_MAGIC "webview_zygote"
#define WEBVIEW_ZYGOTE_UID 1053

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

bool proc_context_match(int pid, std::string_view context);
void *logcat(void *arg);
extern bool logcat_exit;
