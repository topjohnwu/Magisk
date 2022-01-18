#include <sys/types.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <set>

#include <magisk.hpp>
#include <utils.hpp>
#include <db.hpp>

#include "deny.hpp"

using namespace std;

static set<pair<string, string>> *deny_set;             /* set of <pkg, process> pair */
static map<int, vector<string_view>> *app_id_proc_map;  /* app ID -> list of process */
static int inotify_fd = -1;

// Locks the variables above
static pthread_mutex_t data_lock = PTHREAD_MUTEX_INITIALIZER;

atomic<bool> denylist_enforced = false;

#define do_kill (zygisk_enabled && denylist_enforced)

static void rebuild_map() {
    app_id_proc_map->clear();
    string data_path(APP_DATA_DIR);
    size_t len = data_path.length();

    // Collect all user IDs
    vector<string> users;
    if (auto dir = open_dir(APP_DATA_DIR)) {
        for (dirent *entry; (entry = xreaddir(dir.get()));) {
            users.emplace_back(entry->d_name);
        }
    } else {
        return;
    }

    string_view prev_pkg;
    struct stat st;
    for (const auto &target : *deny_set) {
        if (target.first == ISOLATED_MAGIC) {
            // Isolated process
            (*app_id_proc_map)[-1].emplace_back(target.second);
        } else if (prev_pkg == target.first) {
            // Optimize the case when it's the same package as previous iteration
            (*app_id_proc_map)[to_app_id(st.st_uid)].emplace_back(target.second);
        } else {
            // Traverse the filesystem to find app ID
            for (const auto &user_id : users) {
                data_path.resize(len);
                data_path += '/';
                data_path += user_id;
                data_path += '/';
                data_path += target.first;
                if (stat(data_path.data(), &st) == 0) {
                    prev_pkg = target.first;
                    (*app_id_proc_map)[to_app_id(st.st_uid)].emplace_back(target.second);
                    break;
                }
            }
        }
    }
}

// Leave /proc fd opened as we're going to read from it repeatedly
static DIR *procfp;

template<class F>
static void crawl_procfs(F &&fn) {
    rewinddir(procfp);
    dirent *dp;
    int pid;
    while ((dp = readdir(procfp))) {
        pid = parse_int(dp->d_name);
        if (pid > 0 && !fn(pid))
            break;
    }
}

template <bool str_op(string_view, string_view)>
static bool proc_name_match(int pid, const char *name) {
    char buf[4019];
    sprintf(buf, "/proc/%d/cmdline", pid);
    if (auto fp = open_file(buf, "re")) {
        fgets(buf, sizeof(buf), fp.get());
        if (str_op(buf, name)) {
            LOGD("denylist: kill PID=[%d] (%s)\n", pid, buf);
            return true;
        }
    }
    return false;
}

static inline bool str_eql(string_view a, string_view b) { return a == b; }

template<bool str_op(string_view, string_view) = &str_eql>
static void kill_process(const char *name, bool multi = false) {
    crawl_procfs([=](int pid) -> bool {
        if (proc_name_match<str_op>(pid, name)) {
            kill(pid, SIGKILL);
            return multi;
        }
        return true;
    });
}

static bool validate(const char *pkg, const char *proc) {
    bool pkg_valid = false;
    bool proc_valid = true;

    if (str_eql(pkg, ISOLATED_MAGIC)) {
        pkg_valid = true;
        for (char c; (c = *proc); ++proc) {
            if (isalnum(c) || c == '_' || c == '.')
                continue;
            if (c == ':')
                break;
            proc_valid = false;
            break;
        }
    } else {
        for (char c; (c = *pkg); ++pkg) {
            if (isalnum(c) || c == '_')
                continue;
            if (c == '.') {
                pkg_valid = true;
                continue;
            }
            pkg_valid = false;
            break;
        }

        for (char c; (c = *proc); ++proc) {
            if (isalnum(c) || c == '_' || c == ':' || c == '.')
                continue;
            proc_valid = false;
            break;
        }
    }
    return pkg_valid && proc_valid;
}

static void add_hide_set(const char *pkg, const char *proc) {
    LOGI("denylist add: [%s/%s]\n", pkg, proc);
    deny_set->emplace(pkg, proc);
    if (!do_kill)
        return;
    if (str_eql(pkg, ISOLATED_MAGIC)) {
        // Kill all matching isolated processes
        kill_process<&str_starts>(proc, true);
    } else {
        kill_process(proc);
    }
}

static void clear_data() {
    delete deny_set;
    delete app_id_proc_map;
    deny_set = nullptr;
    app_id_proc_map = nullptr;
    unregister_poll(inotify_fd, true);
    inotify_fd = -1;
}

static void inotify_handler(pollfd *pfd) {
    union {
        inotify_event event;
        char buf[512];
    } u{};
    read(pfd->fd, u.buf, sizeof(u.buf));
    if (u.event.name == "packages.xml"sv) {
        cached_manager_app_id = -1;
        exec_task([] {
            mutex_guard lock(data_lock);
            rebuild_map();
        });
    }
}

static bool ensure_data() {
    if (app_id_proc_map)
        return true;

    LOGI("denylist: initializing internal data structures\n");

    default_new(deny_set);
    char *err = db_exec("SELECT * FROM denylist", [](db_row &row) -> bool {
        add_hide_set(row["package_name"].data(), row["process"].data());
        return true;
    });
    db_err_cmd(err, goto error);

    default_new(app_id_proc_map);
    rebuild_map();

    inotify_fd = xinotify_init1(IN_CLOEXEC);
    if (inotify_fd < 0) {
        goto error;
    } else {
        // Monitor packages.xml
        inotify_add_watch(inotify_fd, "/data/system", IN_CLOSE_WRITE);
        pollfd inotify_pfd = { inotify_fd, POLLIN, 0 };
        register_poll(&inotify_pfd, inotify_handler);
    }

    return true;

error:
    clear_data();
    return false;
}

static int add_list(const char *pkg, const char *proc) {
    if (proc[0] == '\0')
        proc = pkg;

    if (!validate(pkg, proc))
        return DENYLIST_INVALID_PKG;

    {
        mutex_guard lock(data_lock);
        if (!ensure_data())
            return DAEMON_ERROR;

        for (const auto &hide : *deny_set)
            if (hide.first == pkg && hide.second == proc)
                return DENYLIST_ITEM_EXIST;
        add_hide_set(pkg, proc);
        rebuild_map();
    }

    // Add to database
    char sql[4096];
    snprintf(sql, sizeof(sql),
            "INSERT INTO denylist (package_name, process) VALUES('%s', '%s')", pkg, proc);
    char *err = db_exec(sql);
    db_err_cmd(err, return DAEMON_ERROR);
    return DAEMON_SUCCESS;
}

int add_list(int client) {
    string pkg = read_string(client);
    string proc = read_string(client);
    return add_list(pkg.data(), proc.data());
}

static int rm_list(const char *pkg, const char *proc) {
    {
        mutex_guard lock(data_lock);
        if (!ensure_data())
            return DAEMON_ERROR;

        bool remove = false;
        for (auto it = deny_set->begin(); it != deny_set->end();) {
            if (it->first == pkg && (proc[0] == '\0' || it->second == proc)) {
                remove = true;
                LOGI("denylist rm: [%s/%s]\n", it->first.data(), it->second.data());
                it = deny_set->erase(it);
            } else {
                ++it;
            }
        }
        if (!remove)
            return DENYLIST_ITEM_NOT_EXIST;
        rebuild_map();
    }

    char sql[4096];
    if (proc[0] == '\0')
        snprintf(sql, sizeof(sql), "DELETE FROM denylist WHERE package_name='%s'", pkg);
    else
        snprintf(sql, sizeof(sql),
                "DELETE FROM denylist WHERE package_name='%s' AND process='%s'", pkg, proc);
    char *err = db_exec(sql);
    db_err_cmd(err, return DAEMON_ERROR);
    return DAEMON_SUCCESS;
}

int rm_list(int client) {
    string pkg = read_string(client);
    string proc = read_string(client);
    return rm_list(pkg.data(), proc.data());
}

void ls_list(int client) {
    {
        mutex_guard lock(data_lock);
        if (!ensure_data()) {
            write_int(client, DAEMON_ERROR);
            return;
        }

        write_int(client, DAEMON_SUCCESS);
        for (const auto &hide : *deny_set) {
            write_int(client, hide.first.size() + hide.second.size() + 1);
            xwrite(client, hide.first.data(), hide.first.size());
            xwrite(client, "|", 1);
            xwrite(client, hide.second.data(), hide.second.size());
        }
    }
    write_int(client, 0);
    close(client);
}

static bool str_ends_safe(string_view s, string_view ss) {
    // Never kill webview zygote
    if (s == "webview_zygote")
        return false;
    return str_ends(s, ss);
}

static void update_deny_config() {
    char sql[64];
    sprintf(sql, "REPLACE INTO settings (key,value) VALUES('%s',%d)",
        DB_SETTING_KEYS[DENYLIST_CONFIG], denylist_enforced.load());
    char *err = db_exec(sql);
    db_err(err);
}

int enable_deny() {
    if (denylist_enforced) {
        return DAEMON_SUCCESS;
    } else {
        mutex_guard lock(data_lock);

        if (access("/proc/self/ns/mnt", F_OK) != 0) {
            LOGW("The kernel does not support mount namespace\n");
            return DENY_NO_NS;
        }

        if (procfp == nullptr && (procfp = opendir("/proc")) == nullptr)
            return DAEMON_ERROR;

        LOGI("* Enable DenyList\n");

        denylist_enforced = true;

        if (!ensure_data()) {
            denylist_enforced = false;
            return DAEMON_ERROR;
        }

        // On Android Q+, also kill blastula pool and all app zygotes
        if (SDK_INT >= 29 && zygisk_enabled) {
            kill_process("usap32", true);
            kill_process("usap64", true);
            kill_process<&str_ends_safe>("_zygote", true);
        }
    }

    update_deny_config();
    return DAEMON_SUCCESS;
}

int disable_deny() {
    if (denylist_enforced) {
        denylist_enforced = false;
        LOGI("* Disable DenyList\n");

        mutex_guard lock(data_lock);
        clear_data();
    }
    update_deny_config();
    return DAEMON_SUCCESS;
}

void initialize_denylist() {
    if (!denylist_enforced) {
        db_settings dbs;
        get_db_settings(dbs, DENYLIST_CONFIG);
        if (dbs[DENYLIST_CONFIG])
            enable_deny();
    }
}

bool is_deny_target(int uid, string_view process) {
    mutex_guard lock(data_lock);
    if (!ensure_data())
        return false;

    int app_id = to_app_id(uid);
    if (app_id >= 90000) {
        // Isolated processes
        auto it = app_id_proc_map->find(-1);
        if (it == app_id_proc_map->end())
            return false;

        for (const auto &s : it->second) {
            if (str_starts(process, s))
                return true;
        }
    } else {
        auto it = app_id_proc_map->find(app_id);
        if (it == app_id_proc_map->end())
            return false;

        for (const auto &s : it->second) {
            if (s == process)
                return true;
        }
    }
    return false;
}
