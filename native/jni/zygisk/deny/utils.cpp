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

// For the following data structures:
// If package name == ISOLATED_MAGIC, or app ID == -1, it means isolated service

// Package name -> list of process names
static unique_ptr<map<string, set<string, StringCmp>, StringCmp>> pkg_to_procs_;
#define pkg_to_procs (*pkg_to_procs_)

// app ID -> list of pkg names (string_view points to a pkg_to_procs key)
static unique_ptr<map<int, set<string_view>>> app_id_to_pkgs_;
#define app_id_to_pkgs (*app_id_to_pkgs_)

// Locks the data structures above
static pthread_mutex_t data_lock = PTHREAD_MUTEX_INITIALIZER;

atomic<bool> denylist_enforced = false;

#define do_kill (zygisk_enabled && denylist_enforced)

static unsigned long long pkg_xml_ino = 0;

static void rescan_apps() {
    {
        struct stat st{};
        stat("/data/system/packages.xml", &st);
        if (pkg_xml_ino == st.st_ino) {
            // Packages has not changed, do not rescan
            return;
        }
        pkg_xml_ino = st.st_ino;
    }

    LOGD("denylist: rescanning apps\n");

    app_id_to_pkgs.clear();
    cached_manager_app_id = -1;

    auto data_dir = xopen_dir(APP_DATA_DIR);
    if (!data_dir)
        return;
    dirent *entry;
    while ((entry = xreaddir(data_dir.get()))) {
        // For each user
        int dfd = xopenat(dirfd(data_dir.get()), entry->d_name, O_RDONLY);
        if (auto dir = xopen_dir(dfd)) {
            while ((entry = xreaddir(dir.get()))) {
                // For each package
                struct stat st{};
                xfstatat(dfd, entry->d_name, &st, 0);
                int app_id = to_app_id(st.st_uid);
                if (app_id_to_pkgs.contains(app_id)) {
                    // This app ID has been handled
                    continue;
                }
                if (auto it = pkg_to_procs.find(entry->d_name); it != pkg_to_procs.end()) {
                    app_id_to_pkgs[app_id].insert(it->first);
                }
            }
        } else {
            close(dfd);
        }
    }
}

static void update_pkg_uid(const string &pkg, bool remove) {
    auto data_dir = xopen_dir(APP_DATA_DIR);
    if (!data_dir)
        return;
    dirent *entry;
    struct stat st{};
    char buf[PATH_MAX] = {0};
    // For each user
    while ((entry = xreaddir(data_dir.get()))) {
        snprintf(buf, sizeof(buf), "%s/%s", entry->d_name, pkg.data());
        if (fstatat(dirfd(data_dir.get()), buf, &st, 0) == 0) {
            int app_id = to_app_id(st.st_uid);
            if (remove) {
                if (auto it = app_id_to_pkgs.find(app_id); it != app_id_to_pkgs.end()) {
                    it->second.erase(pkg);
                    if (it->second.empty()) {
                        app_id_to_pkgs.erase(it);
                    }
                }
            } else {
                app_id_to_pkgs[app_id].insert(pkg);
            }
            break;
        }
    }
}

// Leave /proc fd opened as we're going to read from it repeatedly
static DIR *procfp;

template<class F>
static void crawl_procfs(const F &fn) {
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

static auto add_hide_set(const char *pkg, const char *proc) {
    auto p = pkg_to_procs[pkg].emplace(proc);
    if (!p.second)
        return p;
    LOGI("denylist add: [%s/%s]\n", pkg, proc);
    if (!do_kill)
        return p;
    if (str_eql(pkg, ISOLATED_MAGIC)) {
        // Kill all matching isolated processes
        kill_process<&str_starts>(proc, true);
    } else {
        kill_process(proc);
    }
    return p;
}

static void clear_data() {
    pkg_to_procs_.reset(nullptr);
    app_id_to_pkgs_.reset(nullptr);
}

static bool ensure_data() {
    if (pkg_to_procs_)
        return true;

    LOGI("denylist: initializing internal data structures\n");

    default_new(pkg_to_procs_);
    char *err = db_exec("SELECT * FROM denylist", [](db_row &row) -> bool {
        add_hide_set(row["package_name"].data(), row["process"].data());
        return true;
    });
    db_err_cmd(err, goto error)

    default_new(app_id_to_pkgs_);
    rescan_apps();

    return true;

error:
    clear_data();
    return false;
}

static DenyResponse add_list(const char *pkg, const char *proc) {
    if (proc[0] == '\0')
        proc = pkg;

    if (!validate(pkg, proc))
        return DenyResponse::INVALID_PKG;

    {
        mutex_guard lock(data_lock);
        if (!ensure_data())
            return DenyResponse::ERROR;
        auto p = add_hide_set(pkg, proc);
        if (!p.second)
            return DenyResponse::ITEM_EXIST;
        update_pkg_uid(*p.first, false);
    }

    // Add to database
    char sql[4096];
    snprintf(sql, sizeof(sql),
            "INSERT INTO denylist (package_name, process) VALUES('%s', '%s')", pkg, proc);
    char *err = db_exec(sql);
    db_err_cmd(err, return DenyResponse::ERROR)
    return DenyResponse::OK;
}

DenyResponse add_list(int client) {
    string pkg = read_string(client);
    string proc = read_string(client);
    return add_list(pkg.data(), proc.data());
}

static DenyResponse rm_list(const char *pkg, const char *proc) {
    {
        mutex_guard lock(data_lock);
        if (!ensure_data())
            return DenyResponse::ERROR;

        bool remove = false;

        auto it = pkg_to_procs.find(pkg);
        if (it != pkg_to_procs.end()) {
            if (proc[0] == '\0') {
                update_pkg_uid(it->first, true);
                pkg_to_procs.erase(it);
                remove = true;
                LOGI("denylist rm: [%s]\n", pkg);
            } else if (it->second.erase(proc) != 0) {
                remove = true;
                LOGI("denylist rm: [%s/%s]\n", pkg, proc);
                if (it->second.empty()) {
                    update_pkg_uid(it->first, true);
                    pkg_to_procs.erase(it);
                }
            }
        }

        if (!remove)
            return DenyResponse::ITEM_NOT_EXIST;
    }

    char sql[4096];
    if (proc[0] == '\0')
        snprintf(sql, sizeof(sql), "DELETE FROM denylist WHERE package_name='%s'", pkg);
    else
        snprintf(sql, sizeof(sql),
                "DELETE FROM denylist WHERE package_name='%s' AND process='%s'", pkg, proc);
    char *err = db_exec(sql);
    db_err_cmd(err, return DenyResponse::ERROR)
    return DenyResponse::OK;
}

DenyResponse rm_list(int client) {
    string pkg = read_string(client);
    string proc = read_string(client);
    return rm_list(pkg.data(), proc.data());
}

void ls_list(int client) {
    {
        mutex_guard lock(data_lock);
        if (!ensure_data()) {
            write_int(client, static_cast<int>(DenyResponse::ERROR));
            return;
        }

        write_int(client,static_cast<int>(DenyResponse::OK));

        for (const auto &[pkg, procs] : pkg_to_procs) {
            for (const auto &proc : procs) {
                write_int(client, pkg.size() + proc.size() + 1);
                xwrite(client, pkg.data(), pkg.size());
                xwrite(client, "|", 1);
                xwrite(client, proc.data(), proc.size());
            }
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

DenyResponse enable_deny() {
    if (denylist_enforced) {
        return DenyResponse::OK;
    } else {
        mutex_guard lock(data_lock);

        if (access("/proc/self/ns/mnt", F_OK) != 0) {
            LOGW("The kernel does not support mount namespace\n");
            return DenyResponse::NO_NS;
        }

        if (procfp == nullptr && (procfp = opendir("/proc")) == nullptr)
            return DenyResponse::ERROR;

        LOGI("* Enable DenyList\n");

        denylist_enforced = true;

        if (!ensure_data()) {
            denylist_enforced = false;
            return DenyResponse::ERROR;
        }

        // On Android Q+, also kill blastula pool and all app zygotes
        if (SDK_INT >= 29 && zygisk_enabled) {
            kill_process("usap32", true);
            kill_process("usap64", true);
            kill_process<&str_ends_safe>("_zygote", true);
        }
    }

    update_deny_config();
    return DenyResponse::OK;
}

DenyResponse disable_deny() {
    if (denylist_enforced) {
        denylist_enforced = false;
        LOGI("* Disable DenyList\n");
    }
    update_deny_config();
    return DenyResponse::OK;
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

    rescan_apps();

    int app_id = to_app_id(uid);
    if (app_id >= 90000) {
        if (auto it = pkg_to_procs.find(ISOLATED_MAGIC); it != pkg_to_procs.end()) {
            for (const auto &s : it->second) {
                if (str_starts(process, s))
                    return true;
            }
        }
        return false;
    } else {
        auto it = app_id_to_pkgs.find(app_id);
        if (it == app_id_to_pkgs.end())
            return false;
        for (const auto &pkg : it->second) {
            if (pkg_to_procs.find(pkg)->second.count(process))
                return true;
        }
    }
    return false;
}
