#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/mount.h>

#include <magisk.hpp>
#include <base.hpp>
#include <selinux.hpp>

#include "su.hpp"
#include "pts.hpp"

using namespace std;

static pthread_mutex_t cache_lock = PTHREAD_MUTEX_INITIALIZER;
static shared_ptr<su_info> cached;

su_info::su_info(int uid) :
uid(uid), eval_uid(-1), access(DEFAULT_SU_ACCESS), mgr_uid(-1),
timestamp(0), _lock(PTHREAD_MUTEX_INITIALIZER) {}

su_info::~su_info() {
    pthread_mutex_destroy(&_lock);
}

mutex_guard su_info::lock() {
    return mutex_guard(_lock);
}

bool su_info::is_fresh() {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long current = ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
    return current - timestamp < 3000;  /* 3 seconds */
}

void su_info::refresh() {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    timestamp = ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

void su_info::check_db() {
    eval_uid = uid;
    get_db_settings(cfg);

    // Check multiuser settings
    switch (cfg[SU_MULTIUSER_MODE]) {
    case MULTIUSER_MODE_OWNER_ONLY:
        if (to_user_id(uid) != 0) {
            eval_uid = -1;
            access = NO_SU_ACCESS;
        }
        break;
    case MULTIUSER_MODE_OWNER_MANAGED:
        eval_uid = to_app_id(uid);
        break;
    case MULTIUSER_MODE_USER:
    default:
        break;
    }

    if (eval_uid > 0) {
        char query[256], *err;
        ssprintf(query, sizeof(query),
            "SELECT policy, logging, notification FROM policies "
            "WHERE uid=%d AND (until=0 OR until>%li)", eval_uid, time(nullptr));
        err = db_exec(query, [&](db_row &row) -> bool {
            access.policy = (policy_t) parse_int(row["policy"]);
            access.log = parse_int(row["logging"]);
            access.notify = parse_int(row["notification"]);
            LOGD("magiskdb: query policy=[%d] log=[%d] notify=[%d]\n",
                 access.policy, access.log, access.notify);
            return true;
        });
        db_err_cmd(err, return);
    }

    // We need to check our manager
    if (access.log || access.notify) {
        check_pkg_refresh();
        mgr_uid = get_manager(to_user_id(eval_uid), &mgr_pkg, true);
    }
}

bool uid_granted_root(int uid) {
    if (uid == AID_ROOT)
        return true;

    db_settings cfg;
    get_db_settings(cfg);

    // Check user root access settings
    switch (cfg[ROOT_ACCESS]) {
    case ROOT_ACCESS_DISABLED:
        return false;
    case ROOT_ACCESS_APPS_ONLY:
        if (uid == AID_SHELL)
            return false;
        break;
    case ROOT_ACCESS_ADB_ONLY:
        if (uid != AID_SHELL)
            return false;
        break;
    case ROOT_ACCESS_APPS_AND_ADB:
        break;
    }

    // Check multiuser settings
    switch (cfg[SU_MULTIUSER_MODE]) {
    case MULTIUSER_MODE_OWNER_ONLY:
        if (to_user_id(uid) != 0)
            return false;
        break;
    case MULTIUSER_MODE_OWNER_MANAGED:
        uid = to_app_id(uid);
        break;
    case MULTIUSER_MODE_USER:
    default:
        break;
    }

    bool granted = false;

    char query[256], *err;
    ssprintf(query, sizeof(query),
        "SELECT policy FROM policies WHERE uid=%d AND (until=0 OR until>%li)",
        uid, time(nullptr));
    err = db_exec(query, [&](db_row &row) -> bool {
        granted = parse_int(row["policy"]) == ALLOW;
        return true;
    });
    db_err_cmd(err, return false);

    return granted;
}

void prune_su_access() {
    cached.reset();
    vector<bool> app_no_list = get_app_no_list();
    vector<int> rm_uids;
    char query[256], *err;
    strscpy(query, "SELECT uid FROM policies", sizeof(query));
    err = db_exec(query, [&](db_row &row) -> bool {
        int uid = parse_int(row["uid"]);
        int app_id = to_app_id(uid);
        if (app_id >= AID_APP_START && app_id <= AID_APP_END) {
            int app_no = app_id - AID_APP_START;
            if (app_no >= app_no_list.size() || !app_no_list[app_no]) {
                // The app_id is no longer installed
                rm_uids.push_back(uid);
            }
        }
        return true;
    });
    db_err_cmd(err, return);

    for (int uid : rm_uids) {
        ssprintf(query, sizeof(query), "DELETE FROM policies WHERE uid == %d", uid);
        // Don't care about errors
        db_exec(query);
    }
}

static shared_ptr<su_info> get_su_info(unsigned uid) {
    LOGD("su: request from uid=[%d]\n", uid);

    if (uid == AID_ROOT) {
        auto info = make_shared<su_info>(uid);
        info->access = SILENT_SU_ACCESS;
        return info;
    }

    shared_ptr<su_info> info;
    {
        mutex_guard lock(cache_lock);
        if (!cached || cached->uid != uid || !cached->is_fresh())
            cached = make_shared<su_info>(uid);
        cached->refresh();
        info = cached;
    }

    mutex_guard lock = info->lock();

    if (info->access.policy == QUERY) {
        // Not cached, get data from database
        info->check_db();

        // If it's the manager, allow it silently
        if (to_app_id(info->uid) == to_app_id(info->mgr_uid)) {
            info->access = SILENT_SU_ACCESS;
            return info;
        }

        // Check su access settings
        switch (info->cfg[ROOT_ACCESS]) {
            case ROOT_ACCESS_DISABLED:
                LOGW("Root access is disabled!\n");
                info->access = NO_SU_ACCESS;
                break;
            case ROOT_ACCESS_ADB_ONLY:
                if (info->uid != AID_SHELL) {
                    LOGW("Root access limited to ADB only!\n");
                    info->access = NO_SU_ACCESS;
                }
                break;
            case ROOT_ACCESS_APPS_ONLY:
                if (info->uid == AID_SHELL) {
                    LOGW("Root access is disabled for ADB!\n");
                    info->access = NO_SU_ACCESS;
                }
                break;
            case ROOT_ACCESS_APPS_AND_ADB:
            default:
                break;
        }

        if (info->access.policy != QUERY)
            return info;

        // If still not determined, check if manager exists
        if (info->mgr_uid < 0) {
            info->access = NO_SU_ACCESS;
            return info;
        }
    }
    return info;
}

// Set effective uid back to root, otherwise setres[ug]id will fail if uid isn't root
static void set_identity(unsigned uid) {
    if (seteuid(0)) {
        PLOGE("seteuid (root)");
    }
    if (setresgid(uid, uid, uid)) {
        PLOGE("setresgid (%u)", uid);
    }
    if (setresuid(uid, uid, uid)) {
        PLOGE("setresuid (%u)", uid);
    }
}

void su_daemon_handler(int client, const sock_cred *cred) {
    LOGD("su: request from pid=[%d], client=[%d]\n", cred->pid, client);

    su_context ctx = {
        .info = get_su_info(cred->uid),
        .req = su_request(),
        .pid = cred->pid
    };

    // Read su_request
    if (xxread(client, &ctx.req, sizeof(su_req_base)) < 0
        || !read_string(client, ctx.req.shell)
        || !read_string(client, ctx.req.command)) {
        LOGW("su: remote process probably died, abort\n");
        ctx.info.reset();
        write_int(client, DENY);
        close(client);
        return;
    }

    // If still not determined, ask manager
    if (ctx.info->access.policy == QUERY) {
        int fd = app_request(ctx);
        if (fd < 0) {
            ctx.info->access.policy = DENY;
        } else {
            int ret = read_int_be(fd);
            ctx.info->access.policy = ret < 0 ? DENY : static_cast<policy_t>(ret);
            close(fd);
        }
    }

    if (ctx.info->access.log)
        app_log(ctx);
    else if (ctx.info->access.notify)
        app_notify(ctx);

    // Fail fast
    if (ctx.info->access.policy == DENY) {
        LOGW("su: request rejected (%u)\n", ctx.info->uid);
        ctx.info.reset();
        write_int(client, DENY);
        close(client);
        return;
    }

    // Fork a child root process
    //
    // The child process will need to setsid, open a pseudo-terminal
    // if needed, and eventually exec shell.
    // The parent process will wait for the result and
    // send the return code back to our client.

    if (int child = xfork(); child) {
        ctx.info.reset();

        // Wait result
        LOGD("su: waiting child pid=[%d]\n", child);
        int status, code;

        if (waitpid(child, &status, 0) > 0)
            code = WEXITSTATUS(status);
        else
            code = -1;

        LOGD("su: return code=[%d]\n", code);
        write(client, &code, sizeof(code));
        close(client);
        return;
    }

    LOGD("su: fork handler\n");

    // Abort upon any error occurred
    exit_on_error(true);

    // ack
    write_int(client, 0);

    // Become session leader
    xsetsid();

    // The FDs for each of the streams
    int infd = recv_fd(client);
    int outfd = recv_fd(client);
    int errfd = recv_fd(client);

    // App need a PTY
    if (read_int(client)) {
        string pts;
        string ptmx;
        auto magiskpts = MAGISKTMP + "/" SHELLPTS;
        if (access(magiskpts.data(), F_OK)) {
            pts = "/dev/pts";
            ptmx = "/dev/ptmx";
        } else {
            pts = magiskpts;
            ptmx = magiskpts + "/ptmx";
        }
        int ptmx_fd = xopen(ptmx.data(), O_RDWR);
        grantpt(ptmx_fd);
        unlockpt(ptmx_fd);
        int pty_num = get_pty_num(ptmx_fd);
        if (pty_num < 0) {
            // Kernel issue? Fallback to /dev/pts
            close(ptmx_fd);
            pts = "/dev/pts";
            ptmx_fd = xopen("/dev/ptmx", O_RDWR);
            grantpt(ptmx_fd);
            unlockpt(ptmx_fd);
            pty_num = get_pty_num(ptmx_fd);
        }
        send_fd(client, ptmx_fd);
        close(ptmx_fd);

        string pts_slave = pts + "/" + to_string(pty_num);
        LOGD("su: pts_slave=[%s]\n", pts_slave.data());

        // Opening the TTY has to occur after the
        // fork() and setsid() so that it becomes
        // our controlling TTY and not the daemon's
        int ptsfd = xopen(pts_slave.data(), O_RDWR);

        if (infd < 0)
            infd = ptsfd;
        if (outfd < 0)
            outfd = ptsfd;
        if (errfd < 0)
            errfd = ptsfd;
    }

    // Swap out stdin, stdout, stderr
    xdup2(infd, STDIN_FILENO);
    xdup2(outfd, STDOUT_FILENO);
    xdup2(errfd, STDERR_FILENO);

    close(infd);
    close(outfd);
    close(errfd);
    close(client);

    // Handle namespaces
    if (ctx.req.mount_master)
        ctx.info->cfg[SU_MNT_NS] = NAMESPACE_MODE_GLOBAL;
    switch (ctx.info->cfg[SU_MNT_NS]) {
        case NAMESPACE_MODE_GLOBAL:
            LOGD("su: use global namespace\n");
            break;
        case NAMESPACE_MODE_REQUESTER:
            LOGD("su: use namespace of pid=[%d]\n", ctx.pid);
            if (switch_mnt_ns(ctx.pid))
                LOGD("su: setns failed, fallback to global\n");
            break;
        case NAMESPACE_MODE_ISOLATE:
            LOGD("su: use new isolated namespace\n");
            switch_mnt_ns(ctx.pid);
            xunshare(CLONE_NEWNS);
            xmount(nullptr, "/", nullptr, MS_PRIVATE | MS_REC, nullptr);
            break;
    }

    const char *argv[4] = { nullptr };

    argv[0] = ctx.req.login ? "-" : ctx.req.shell.data();

    if (!ctx.req.command.empty()) {
        argv[1] = "-c";
        argv[2] = ctx.req.command.data();
    }

    // Setup environment
    umask(022);
    char path[32];
    ssprintf(path, sizeof(path), "/proc/%d/cwd", ctx.pid);
    char cwd[4096];
    if (realpath(path, cwd, sizeof(cwd)) > 0)
        chdir(cwd);
    ssprintf(path, sizeof(path), "/proc/%d/environ", ctx.pid);
    auto env = full_read(path);
    clearenv();
    for (size_t pos = 0; pos < env.size(); ++pos) {
        putenv(env.data() + pos);
        pos = env.find_first_of('\0', pos);
        if (pos == std::string::npos)
            break;
    }
    if (!ctx.req.keepenv) {
        struct passwd *pw;
        pw = getpwuid(ctx.req.uid);
        if (pw) {
            setenv("HOME", pw->pw_dir, 1);
            setenv("USER", pw->pw_name, 1);
            setenv("LOGNAME", pw->pw_name, 1);
            setenv("SHELL", ctx.req.shell.data(), 1);
        }
    }

    // Unblock all signals
    sigset_t block_set;
    sigemptyset(&block_set);
    sigprocmask(SIG_SETMASK, &block_set, nullptr);
    set_identity(ctx.req.uid);
    execvp(ctx.req.shell.data(), (char **) argv);
    fprintf(stderr, "Cannot execute %s: %s\n", ctx.req.shell.data(), strerror(errno));
    PLOGE("exec");
}
