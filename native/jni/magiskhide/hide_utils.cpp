#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

#include <magisk.hpp>
#include <utils.hpp>
#include <db.hpp>

#include "magiskhide.hpp"

using namespace std;

static pthread_t proc_monitor_thread;
static bool hide_state = false;

// This locks the 2 variables above
static pthread_mutex_t hide_state_lock = PTHREAD_MUTEX_INITIALIZER;

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

void set_hide_state(bool state) {
    mutex_guard g(hide_state_lock);
    hide_state = state;
}

template <bool str_op(string_view, string_view)>
static bool proc_name_match(int pid, const char *name) {
    char buf[4019];
    sprintf(buf, "/proc/%d/cmdline", pid);
    if (auto fp = open_file(buf, "re")) {
        fgets(buf, sizeof(buf), fp.get());
        if (str_op(buf, name)) {
            LOGD("hide_utils: kill PID=[%d] (%s)\n", pid, buf);
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
            kill(pid, SIGTERM);
            return multi;
        }
        return true;
    });
}

static bool validate(const char *s) {
    if (strcmp(s, ISOLATED_MAGIC) == 0)
        return true;
    bool dot = false;
    for (char c; (c = *s); ++s) {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '_' || c == ':') {
            continue;
        }
        if (c == '.') {
            dot = true;
            continue;
        }
        return false;
    }
    return dot;
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

    if (!validate(pkg) || !validate(proc))
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
        mutex_guard lock(monitor_lock);
        add_hide_set(pkg, proc);
    }

    return DAEMON_SUCCESS;
}

int add_list(int client) {
    char *pkg = read_string(client);
    char *proc = read_string(client);
    int ret = add_list(pkg, proc);
    free(pkg);
    free(proc);
    if (ret == DAEMON_SUCCESS)
        update_uid_map();
    return ret;
}

static int rm_list(const char *pkg, const char *proc) {
    bool remove = false;
    {
        // Critical region
        mutex_guard lock(monitor_lock);
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
    char *pkg = read_string(client);
    char *proc = read_string(client);
    int ret = rm_list(pkg, proc);
    free(pkg);
    free(proc);
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
#define MICROG_PKG   "org.microg.gms.droidguard"

static bool init_list() {
    LOGD("hide_list: initialize\n");

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
    add_hide_set(MICROG_PKG, SNET_PROC);

    // We also need to hide the default GMS process if MAGISKTMP != /sbin
    // The snet process communicates with the main process and get additional info
    if (MAGISKTMP != "/sbin")
        add_hide_set(GMS_PKG, GMS_PKG);

    update_uid_map();
    return true;
}

void ls_list(int client) {
    FILE *out = fdopen(recv_fd(client), "a");
    for (auto &hide : hide_set)
        fprintf(out, "%s|%s\n", hide.first.data(), hide.second.data());
    fclose(out);
    write_int(client, DAEMON_SUCCESS);
    close(client);
}

static void update_hide_config() {
    char sql[64];
    sprintf(sql, "REPLACE INTO settings (key,value) VALUES('%s',%d)",
            DB_SETTING_KEYS[HIDE_CONFIG], hide_state);
    char *err = db_exec(sql);
    db_err(err);
}

int launch_magiskhide() {
    mutex_guard g(hide_state_lock);

    if (SDK_INT < 19)
        return DAEMON_ERROR;

    if (hide_state)
        return HIDE_IS_ENABLED;

    if (access("/proc/1/ns/mnt", F_OK) != 0)
        return HIDE_NO_NS;

    if (procfp == nullptr && (procfp = opendir("/proc")) == nullptr)
        return DAEMON_ERROR;

    LOGI("* Starting MagiskHide\n");

    // Initialize the mutex lock
    pthread_mutex_init(&monitor_lock, nullptr);

    // Initialize the hide list
    if (!init_list())
        return DAEMON_ERROR;

    hide_sensitive_props();
    if (DAEMON_STATE >= STATE_BOOT_COMPLETE || DAEMON_STATE == STATE_NONE)
        hide_late_sensitive_props();

    // Start monitoring
    void *(*start)(void*) = [](void*) -> void* { proc_monitor(); };
    if (xpthread_create(&proc_monitor_thread, nullptr, start, nullptr))
        return DAEMON_ERROR;

    hide_state = true;
    update_hide_config();
    return DAEMON_SUCCESS;
}

int stop_magiskhide() {
    mutex_guard g(hide_state_lock);

    if (hide_state) {
        LOGI("* Stopping MagiskHide\n");
        pthread_kill(proc_monitor_thread, SIGTERMTHRD);
    }

    hide_state = false;
    update_hide_config();
    return DAEMON_SUCCESS;
}

void auto_start_magiskhide() {
    if (hide_enabled()) {
        pthread_kill(proc_monitor_thread, SIGALRM);
        hide_late_sensitive_props();
    } else if (SDK_INT >= 19) {
        db_settings dbs;
        get_db_settings(dbs, HIDE_CONFIG);
        if (dbs[HIDE_CONFIG])
            launch_magiskhide();
    }
}

void test_proc_monitor() {
    if (procfp == nullptr && (procfp = opendir("/proc")) == nullptr)
        exit(1);
    proc_monitor();
}
