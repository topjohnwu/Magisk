#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <set>

#include <magisk.hpp>
#include <utils.hpp>
#include <db.hpp>

#include "magiskhide.hpp"

using namespace std;

static bool hide_state = false;
static set<pair<string, string>> hide_set;   /* set of <pkg, process> pair */
map<int, vector<string_view>> uid_proc_map;  /* uid -> list of process */

// Locks the variables above
pthread_mutex_t hide_state_lock = PTHREAD_MUTEX_INITIALIZER;

void update_uid_map() {
    mutex_guard lock(hide_state_lock);
    uid_proc_map.clear();
    string data_path(APP_DATA_DIR);
    size_t len = data_path.length();
    auto dir = open_dir(APP_DATA_DIR);
    bool first_iter = true;
    for (dirent *entry; (entry = xreaddir(dir.get()));) {
        data_path.resize(len);
        data_path += '/';
        data_path += entry->d_name;  // multiuser user id
        data_path += '/';
        size_t user_len = data_path.length();
        struct stat st;
        for (auto &hide : hide_set) {
            if (hide.first == ISOLATED_MAGIC) {
                if (!first_iter) continue;
                // Setup isolated processes
                uid_proc_map[-1].emplace_back(hide.second);
            }
            data_path.resize(user_len);
            data_path += hide.first;
            if (stat(data_path.data(), &st))
                continue;
            uid_proc_map[st.st_uid].emplace_back(hide.second);
        }
        first_iter = false;
    }
}

// Leave /proc fd opened as we're going to read from it repeatedly
static DIR *procfp;
void crawl_procfs(const function<bool(int)> &fn) {
    rewinddir(procfp);
    crawl_procfs(procfp, fn);
}

void crawl_procfs(DIR *dir, const function<bool(int)> &fn) {
    struct dirent *dp;
    int pid;
    while ((dp = readdir(dir))) {
        pid = parse_int(dp->d_name);
        if (pid > 0 && !fn(pid))
            break;
    }
}

bool hide_enabled() {
    mutex_guard g(hide_state_lock);
    return hide_state;
}

template <bool str_op(string_view, string_view)>
static bool proc_name_match(int pid, const char *name) {
    char buf[4019];
    sprintf(buf, "/proc/%d/cmdline", pid);
    if (auto fp = open_file(buf, "re")) {
        fgets(buf, sizeof(buf), fp.get());
        if (str_op(buf, name)) {
            LOGD("hide: kill PID=[%d] (%s)\n", pid, buf);
            return true;
        }
    }
    return false;
}

static inline bool str_eql(string_view s, string_view ss) { return s == ss; }

static void kill_process(const char *name, bool multi = false,
        bool (*filter)(int, const char *) = proc_name_match<&str_eql>) {
    crawl_procfs([=](int pid) -> bool {
        if (filter(pid, name)) {
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
    LOGI("hide_list add: [%s/%s]\n", pkg, proc);
    hide_set.emplace(pkg, proc);
    if (strcmp(pkg, ISOLATED_MAGIC) == 0) {
        // Kill all matching isolated processes
        kill_process(proc, true, proc_name_match<&str_starts>);
    } else {
        kill_process(proc);
    }
}

static int add_list(const char *pkg, const char *proc) {
    if (proc[0] == '\0')
        proc = pkg;

    if (!validate(pkg, proc))
        return HIDE_INVALID_PKG;

    for (auto &hide : hide_set)
        if (hide.first == pkg && hide.second == proc)
            return HIDE_ITEM_EXIST;

    // Add to database
    char sql[4096];
    snprintf(sql, sizeof(sql),
            "INSERT INTO hidelist (package_name, process) VALUES('%s', '%s')", pkg, proc);
    char *err = db_exec(sql);
    db_err_cmd(err, return DAEMON_ERROR);

    {
        // Critical region
        mutex_guard lock(hide_state_lock);
        add_hide_set(pkg, proc);
    }

    return DAEMON_SUCCESS;
}

int add_list(int client) {
    string pkg = read_string(client);
    string proc = read_string(client);
    int ret = add_list(pkg.data(), proc.data());
    if (ret == DAEMON_SUCCESS)
        update_uid_map();
    return ret;
}

static int rm_list(const char *pkg, const char *proc) {
    bool remove = false;
    {
        // Critical region
        mutex_guard lock(hide_state_lock);
        for (auto it = hide_set.begin(); it != hide_set.end();) {
            if (it->first == pkg && (proc[0] == '\0' || it->second == proc)) {
                remove = true;
                LOGI("hide_list rm: [%s/%s]\n", it->first.data(), it->second.data());
                it = hide_set.erase(it);
            } else {
                ++it;
            }
        }
    }
    if (!remove)
        return HIDE_ITEM_NOT_EXIST;

    char sql[4096];
    if (proc[0] == '\0')
        snprintf(sql, sizeof(sql), "DELETE FROM hidelist WHERE package_name='%s'", pkg);
    else
        snprintf(sql, sizeof(sql),
                "DELETE FROM hidelist WHERE package_name='%s' AND process='%s'", pkg, proc);
    char *err = db_exec(sql);
    db_err(err);
    return DAEMON_SUCCESS;
}

int rm_list(int client) {
    string pkg = read_string(client);
    string proc = read_string(client);
    int ret = rm_list(pkg.data(), proc.data());
    if (ret == DAEMON_SUCCESS)
        update_uid_map();
    return ret;
}

static bool str_ends_safe(string_view s, string_view ss) {
    // Never kill webview zygote
    if (s == "webview_zygote")
        return false;
    return str_ends(s, ss);
}

#define SNET_PROC    "com.google.android.gms.unstable"
#define GMS_PKG      "com.google.android.gms"

static bool init_list() {
    LOGD("hide: initialize\n");

    char *err = db_exec("SELECT * FROM hidelist", [](db_row &row) -> bool {
        add_hide_set(row["package_name"].data(), row["process"].data());
        return true;
    });
    db_err_cmd(err, return false);

    // If Android Q+, also kill blastula pool and all app zygotes
    if (SDK_INT >= 29) {
        kill_process("usap32", true);
        kill_process("usap64", true);
        kill_process("_zygote", true, proc_name_match<&str_ends_safe>);
    }

    // Add SafetyNet by default
    add_hide_set(GMS_PKG, SNET_PROC);

    // We also need to hide the default GMS process if MAGISKTMP != /sbin
    // The snet process communicates with the main process and get additional info
    if (MAGISKTMP != "/sbin")
        add_hide_set(GMS_PKG, GMS_PKG);

    return true;
}

void ls_list(int client) {
    write_int(client, DAEMON_SUCCESS);
    for (auto &hide : hide_set) {
        write_int(client, hide.first.size() + hide.second.size() + 1);
        xwrite(client, hide.first.data(), hide.first.size());
        xwrite(client, "|", 1);
        xwrite(client, hide.second.data(), hide.second.size());
    }
    write_int(client, 0);
    close(client);
}

static void update_hide_config() {
    char sql[64];
    sprintf(sql, "REPLACE INTO settings (key,value) VALUES('%s',%d)",
            DB_SETTING_KEYS[HIDE_CONFIG], hide_state);
    char *err = db_exec(sql);
    db_err(err);
}

int launch_magiskhide(bool late_props) {
    mutex_guard lock(hide_state_lock);

    if (hide_state)
        return HIDE_IS_ENABLED;

    if (access("/proc/self/ns/mnt", F_OK) != 0)
        return HIDE_NO_NS;

    if (procfp == nullptr && (procfp = opendir("/proc")) == nullptr)
        return DAEMON_ERROR;

    LOGI("* Enable MagiskHide\n");

    // Initialize the hide list
    if (!init_list())
        return DAEMON_ERROR;

    hide_sensitive_props();
    if (late_props)
        hide_late_sensitive_props();

#if !ENABLE_INJECT
    // Start monitoring
    if (new_daemon_thread(&proc_monitor))
        return DAEMON_ERROR;
#endif

    hide_state = true;
    update_hide_config();

    // Unlock here or else we'll be stuck in deadlock
    lock.unlock();

    update_uid_map();
    return DAEMON_SUCCESS;
}

int stop_magiskhide() {
    mutex_guard g(hide_state_lock);

    if (hide_state) {
        LOGI("* Disable MagiskHide\n");
        uid_proc_map.clear();
        hide_set.clear();
#if !ENABLE_INJECT
        pthread_kill(monitor_thread, SIGTERMTHRD);
#endif
    }

    hide_state = false;
    update_hide_config();
    return DAEMON_SUCCESS;
}

void auto_start_magiskhide(bool late_props) {
    if (hide_enabled()) {
#if !ENABLE_INJECT
        pthread_kill(monitor_thread, SIGALRM);
#endif
        hide_late_sensitive_props();
    } else {
        db_settings dbs;
        get_db_settings(dbs, HIDE_CONFIG);
        if (dbs[HIDE_CONFIG])
            launch_magiskhide(late_props);
    }
}

bool is_hide_target(int uid, string_view process, int max_len) {
    mutex_guard lock(hide_state_lock);

    if (uid % 100000 >= 90000) {
        // Isolated processes
        auto it = uid_proc_map.find(-1);
        if (it == uid_proc_map.end())
            return false;

        for (auto &s : it->second) {
            if (s.length() > max_len && process.length() > max_len && str_starts(s, process))
                return true;
            if (str_starts(process, s))
                return true;
        }
    } else {
        auto it = uid_proc_map.find(uid);
        if (it == uid_proc_map.end())
            return false;

        for (auto &s : it->second) {
            if (s.length() > max_len && process.length() > max_len && str_starts(s, process))
                return true;
            if (s == process)
                return true;
        }
    }
    return false;
}

#if !ENABLE_INJECT
void test_proc_monitor() {
    if (procfp == nullptr && (procfp = opendir("/proc")) == nullptr)
        exit(1);
    proc_monitor();
}
#endif

#if ENABLE_INJECT
int check_uid_map(int client) {
    if (!hide_enabled())
        return 0;

    int uid = read_int(client);
    string process = read_string(client);
    return is_hide_target(uid, process) ? 1 : 0;
}
#endif
