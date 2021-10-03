#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <set>

#include <magisk.hpp>
#include <utils.hpp>
#include <selinux.hpp>
#include <db.hpp>

#include "magiskhide.hpp"

using namespace std;

static bool hide_state = false;
static set<pair<int, pair<string, string>>> hide_set;   /* set of <uid, <pkg, process>> pair */
map<int, vector<string_view>> uid_proc_map;  /* uid -> list of process */
static int multiuser_mode = -1;
string system_mnt_type;
string system_root_mnt_type;

// Locks the variables above
pthread_mutex_t hide_state_lock = PTHREAD_MUTEX_INITIALIZER;

void update_uid_map() {
    mutex_guard lock(hide_state_lock);
    uid_proc_map.clear();
    string data_path(APP_DATA_DIR);
    data_path += "/";
    size_t len = data_path.length();
    struct stat st{};
    for (auto &hide : hide_set) {
        data_path.erase(data_path.begin() + len, data_path.end());
        int userid = hide.first / 100000;
        data_path += to_string(userid) + "/" + hide.second.first;
        if (stat(data_path.data(), &st) == 0 && hide.first == st.st_uid) {
            uid_proc_map[st.st_uid].emplace_back(hide.second.second);
        } else {
            LOGW("UID=%d %s not found, remove!\n", hide.first, hide.second.first.data());
            hide_set.erase(hide);
            char sql[4096];
            snprintf(sql, sizeof(sql), "DELETE FROM sulist WHERE uid='%d' AND package_name='%s'",
                     hide.first, hide.second.first.data());
            char *err = db_exec(sql);
            db_err(err);
        }
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
        return false;
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

static void add_hide_set(int uid, const char *pkg, const char *proc) {
    if (multiuser_mode == -1) {
        db_settings dbs;
        get_db_settings(dbs, SU_MULTIUSER_MODE);
        multiuser_mode = dbs[SU_MULTIUSER_MODE];
    }

    if (uid == 2000 || (multiuser_mode != MULTIUSER_MODE_USER && uid >= 100000)) {
        LOGW("su_list init: ignore init UID=%d %s\n", uid, pkg);
        return;
    }

    LOGI("su_list init: UID=%d [%s/%s]\n", uid, pkg, proc);
    hide_set.emplace(uid, make_pair(pkg, proc));
    kill_process(proc);
}

static int add_list(int uid, const char *pkg, const char *proc) {
    if (proc[0] == '\0')
        proc = pkg;

    if (uid % 100000 < 10000 || uid % 100000 > 19999 || !validate(pkg, proc))
        return HIDE_INVALID_PKG;

    for (auto &hide : hide_set)
        if (hide.first == uid && hide.second.first == pkg && hide.second.second == proc)
            return HIDE_ITEM_EXIST;

    // Add to database
    char sql[4096];
    snprintf(sql, sizeof(sql),
             "INSERT INTO sulist (uid, package_name, process, logging, notification) "
             "VALUES('%d', '%s', '%s', 1, 1)", uid, pkg, proc);
    char *err = db_exec(sql);
    db_err_cmd(err, return DAEMON_ERROR);

    {
        // Critical region
        mutex_guard lock(hide_state_lock);
        add_hide_set(uid, pkg, proc);
    }

    return DAEMON_SUCCESS;
}

int add_list(int client) {
    int uid = read_int(client);
    string pkg = read_string(client);
    string proc = read_string(client);
    int ret = add_list(uid, pkg.data(), proc.data());
    if (ret == DAEMON_SUCCESS)
        update_uid_map();
    return ret;
}

static int rm_list(int uid, const char *pkg, const char *proc) {
    bool remove = false;
    {
        // Critical region
        mutex_guard lock(hide_state_lock);
        for (auto it = hide_set.begin(); it != hide_set.end();) {
            if (it->first == uid && it->second.first == pkg &&
                (proc[0] == '\0' || it->second.second == proc)) {
                remove = true;
                LOGI("su_list rm: UID=%d [%s/%s]\n", it->first, it->second.first.data(), it->second.second.data());
                it = hide_set.erase(it);
            } else {
                ++it;
            }
        }
    }
    if (!remove)
        return HIDE_ITEM_NOT_EXIST;

    char sql[4096];
    if (proc[0] == '\0') snprintf(sql, sizeof(sql), "DELETE FROM sulist WHERE uid='%d' AND package_name='%s'", uid, pkg);
    else snprintf(sql, sizeof(sql),"DELETE FROM sulist WHERE uid='%d' AND package_name='%s' AND process='%s'", uid, pkg, proc);
    char *err = db_exec(sql);
    db_err(err);
    return DAEMON_SUCCESS;
}

int rm_list(int client) {
    int uid = read_int(client);
    string pkg = read_string(client);
    string proc = read_string(client);
    int ret = rm_list(uid, pkg.data(), proc.data());
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
    LOGD("su_list: initialize\n");

    char *err = db_exec("SELECT * FROM sulist", [](db_row &row) -> bool {
        add_hide_set(parse_int(row["uid"]), row["package_name"].data(), row["process"].data());
        return true;
    });
    db_err_cmd(err, return false);

    // If Android Q+, also kill blastula pool
    if (SDK_INT >= 29) {
        kill_process("usap32", true);
        kill_process("usap64", true);
    }

    kill_process(SNET_PROC);
    kill_process(GMS_PKG);

    db_strings str;
    struct stat st{};
    get_db_strings(str, SU_MANAGER);
    if (validate_manager(str[SU_MANAGER], 0, &st)) {
        add_hide_set(st.st_uid, str[SU_MANAGER].data(), str[SU_MANAGER].data());
    }

    return true;
}

void ls_list(int client) {
    write_int(client, DAEMON_SUCCESS);
    for (auto &hide : hide_set) {
        string uid = to_string(hide.first);
        write_int(client, uid.size() + hide.second.first.size() + hide.second.second.size() + 2);
        xwrite(client, uid.data(), uid.size());
        xwrite(client, "|", 1);
        xwrite(client, hide.second.first.data(), hide.second.first.size());
        xwrite(client, "|", 1);
        xwrite(client, hide.second.second.data(), hide.second.second.size());
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

static void cp(const char *src, const char *dest) {
    file_attr a{};
    unlink(dest);
    int sfd = xopen(src, O_RDONLY | O_CLOEXEC);
    int dfd = xopen(dest, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0);
    fgetattr(sfd, &a);
    xsendfile(dfd, sfd, nullptr, a.st.st_size);
    close(sfd);
    close(dfd);
    setattr(dest, &a);
}

static void copy_magisk_tmp() {
    string tmp_dir;
    char buf[8];
    gen_rand_str(buf, sizeof(buf));
    tmp_dir = "/dev/"s + buf;
    xmkdir(tmp_dir.data(), 0);
    setfilecon(tmp_dir.data(), "u:object_r:tmpfs:s0");

    SUMODULE = tmp_dir;
    chdir(tmp_dir.data());

    xmkdir(INTLROOT, 0755);
    xmkdir(MIRRDIR, 0);
    xmkdir(BLOCKDIR, 0);

    for (auto file : {"magisk", "magiskinit"}) {
        auto src = MAGISKTMP + "/"s + file;
        auto dest = tmp_dir + "/"s + file;
        cp(src.data(), dest.data());
        if (file == "magisk"sv) setfilecon(dest.data(), "u:object_r:" SEPOL_EXEC_TYPE ":s0");
    }

    parse_mnt("/proc/mounts", [&](mntent *me) {
        struct stat st{};
        if ((me->mnt_dir == string_view("/system")) && me->mnt_type != "tmpfs"sv &&
            lstat(me->mnt_dir, &st) == 0) {
            mknod(BLOCKDIR "/system", S_IFBLK | 0600, st.st_dev);
            xmkdir(MIRRDIR "/system", 0755);
            system_mnt_type = me->mnt_type;
            return false;
        }
        return true;
    });
    if (access(MIRRDIR "/system", F_OK) != 0) {
        xsymlink("./system_root/system", MIRRDIR "/system");
        parse_mnt("/proc/mounts", [&](mntent *me) {
            struct stat st{};
            if ((me->mnt_dir == string_view("/")) && me->mnt_type != "rootfs"sv &&
                stat("/", &st) == 0) {
                mknod(BLOCKDIR "/system_root", S_IFBLK | 0600, st.st_dev);
                xmkdir(MIRRDIR "/system_root", 0755);
                system_root_mnt_type = me->mnt_type;
                return false;
            }
            return true;
        });
    }

    chdir("/");
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

    copy_magisk_tmp();

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
    rm_rf(SUMODULE.data());
    return DAEMON_SUCCESS;
}

void auto_start_magiskhide(bool late_props) {
    if (hide_enabled()) {
        return;
    } else {
        db_settings dbs;
        get_db_settings(dbs, HIDE_CONFIG);
        if (dbs[HIDE_CONFIG])
            launch_magiskhide(late_props);
    }
}

bool is_hide_target(int uid, string_view process, int max_len) {
    mutex_guard lock(hide_state_lock);

    if (multiuser_mode == MULTIUSER_MODE_OWNER_ONLY && uid >= 100000) {
        return false;
    }

    if (uid % 100000 >= 90000) {
        // Isolated processes
            return false;
    } else {
        auto it = uid_proc_map.find(multiuser_mode == MULTIUSER_MODE_OWNER_MANAGED ? uid % 100000 : uid);
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
