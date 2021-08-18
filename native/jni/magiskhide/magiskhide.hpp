#pragma once

#include <pthread.h>
#include <string_view>
#include <functional>
#include <map>

#include <daemon.hpp>

#define ISOLATED_MAGIC "isolated"

// CLI entries
int enable_hide();
int disable_hide();
int add_list(int client);
int rm_list(int client);
void ls_list(int client);

// Utility functions
bool hide_enabled();
bool is_hide_target(int uid, std::string_view process);

// Hide policies
void hide_daemon(int pid);
void hide_unmount(int pid = -1);

enum {
    ENABLE_HIDE,
    DISABLE_HIDE,
    ADD_LIST,
    RM_LIST,
    LS_LIST,
    HIDE_STATUS,
};

enum {
    HIDE_IS_ENABLED = DAEMON_LAST,
    HIDE_NOT_ENABLED,
    HIDE_ITEM_EXIST,
    HIDE_ITEM_NOT_EXIST,
    HIDE_NO_NS,
    HIDE_INVALID_PKG
};
