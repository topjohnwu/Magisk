#pragma once

#include <sys/socket.h>
#include <poll.h>
#include <string>
#include <atomic>
#include <functional>

#include <base.hpp>

#include "../core-rs.hpp"

#define AID_ROOT   0
#define AID_SHELL  2000
#define AID_USER_OFFSET 100000

#define to_app_id(uid)  (uid % AID_USER_OFFSET)
#define to_user_id(uid) (uid / AID_USER_OFFSET)

// Return codes for daemon
enum class RespondCode : int {
    ERROR = -1,
    OK = 0,
    ROOT_REQUIRED,
    ACCESS_DENIED,
    END
};

struct ModuleInfo;

// Daemon
int connect_daemon(int req, bool create = false);
const char *get_magisk_tmp();
void unlock_blocks();
bool setup_magisk_env();
bool check_key_combo();

// Zygisk daemon
rust::Str get_zygisk_lib_name();
void set_zygisk_prop();
void restore_zygisk_prop();

// Sockets
struct sock_cred : public ucred {
    std::string context;
};

template<typename T> requires(std::is_trivially_copyable_v<T>)
T read_any(int fd) {
    T val;
    if (xxread(fd, &val, sizeof(val)) != sizeof(val))
        return -1;
    return val;
}

template<typename T> requires(std::is_trivially_copyable_v<T>)
void write_any(int fd, T val) {
    if (fd < 0) return;
    xwrite(fd, &val, sizeof(val));
}

bool get_client_cred(int fd, sock_cred *cred);
static inline int read_int(int fd) { return read_any<int>(fd); }
static inline void write_int(int fd, int val) { write_any(fd, val); }
std::string read_string(int fd);
bool read_string(int fd, std::string &str);
void write_string(int fd, std::string_view str);

template<typename T> requires(std::is_trivially_copyable_v<T>)
void write_vector(int fd, const std::vector<T> &vec) {
    write_int(fd, vec.size());
    xwrite(fd, vec.data(), vec.size() * sizeof(T));
}

template<typename T> requires(std::is_trivially_copyable_v<T>)
bool read_vector(int fd, std::vector<T> &vec) {
    int size = read_int(fd);
    vec.resize(size);
    return xread(fd, vec.data(), size * sizeof(T)) == size * sizeof(T);
}

// Poll control
using poll_callback = void(*)(pollfd*);
void register_poll(const pollfd *pfd, poll_callback callback);
void unregister_poll(int fd, bool auto_close);
void clear_poll();

// Thread pool
void init_thread_pool();
void exec_task(std::function<void()> &&task);

// Daemon handlers
void denylist_handler(int client, const sock_cred *cred);

// Module stuffs
void disable_modules();
void remove_modules();

// Scripting
void install_apk(rust::Utf8CStr apk);
void uninstall_pkg(rust::Utf8CStr pkg);
void exec_common_scripts(rust::Utf8CStr stage);
void exec_module_scripts(rust::Utf8CStr stage, const rust::Vec<ModuleInfo> &module_list);
void exec_script(const char *script);
void clear_pkg(const char *pkg, int user_id);
[[noreturn]] void install_module(const char *file);

// Denylist
extern std::atomic<bool> denylist_enforced;
int denylist_cli(int argc, char **argv);
void initialize_denylist();
void scan_deny_apps();
bool is_deny_target(int uid, std::string_view process);
void revert_unmount(int pid = -1) noexcept;
void update_deny_flags(int uid, rust::Str process, uint32_t &flags);

// MagiskSU
void exec_root_shell(int client, int pid, SuRequest &req, MntNsMode mode);
void app_log(const SuAppRequest &info, SuPolicy policy, bool notify);
void app_notify(const SuAppRequest &info, SuPolicy policy);
int app_request(const SuAppRequest &info);

// Rust bindings
static inline rust::Utf8CStr get_magisk_tmp_rs() { return get_magisk_tmp(); }
static inline rust::String resolve_preinit_dir_rs(rust::Utf8CStr base_dir) {
    return resolve_preinit_dir(base_dir.c_str());
}
