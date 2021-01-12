#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/mount.h>

#include <daemon.hpp>
#include <utils.hpp>
#include <selinux.hpp>
#include <db.hpp>

#include "su.hpp"
#include "pts.hpp"

using namespace std;

static pthread_mutex_t cache_lock = PTHREAD_MUTEX_INITIALIZER;
static shared_ptr<su_info> cached;

su_info::su_info(unsigned uid) :
        uid(uid), access(DEFAULT_SU_ACCESS), mgr_st({}),
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

static void database_check(const shared_ptr<su_info> &info) {
    int uid = info->uid;
    get_db_settings(info->cfg);
    get_db_strings(info->str);

    // Check multiuser settings
    switch (info->cfg[SU_MULTIUSER_MODE]) {
        case MULTIUSER_MODE_OWNER_ONLY:
            if (info->uid / 100000) {
                uid = -1;
                info->access = NO_SU_ACCESS;
            }
            break;
        case MULTIUSER_MODE_OWNER_MANAGED:
            uid = info->uid % 100000;
            break;
        case MULTIUSER_MODE_USER:
        default:
            break;
    }

    if (uid > 0)
        get_uid_policy(info->access, uid);

    // We need to check our manager
    if (info->access.log || info->access.notify)
        validate_manager(info->str[SU_MANAGER], uid / 100000, &info->mgr_st);
}

static shared_ptr<su_info> get_su_info(unsigned uid) {
    LOGD("su: request from uid=[%d]\n", uid);

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
        database_check(info);

        // If it's root or the manager, allow it silently
        if (info->uid == UID_ROOT || (info->uid % 100000) == (info->mgr_st.st_uid % 100000)) {
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
                if (info->uid != UID_SHELL) {
                    LOGW("Root access limited to ADB only!\n");
                    info->access = NO_SU_ACCESS;
                }
                break;
            case ROOT_ACCESS_APPS_ONLY:
                if (info->uid == UID_SHELL) {
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
        if (info->str[SU_MANAGER].empty()) {
            info->access = NO_SU_ACCESS;
            return info;
        }
    } else {
        return info;
    }

    // If still not determined, ask manager
    int fd = app_request(info);
    if (fd < 0) {
        info->access.policy = DENY;
    } else {
        int ret = read_int_be(fd);
        info->access.policy = ret < 0 ? DENY : static_cast<policy_t>(ret);
        close(fd);
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

void su_daemon_handler(int client, struct ucred *credential) {
    LOGD("su: request from pid=[%d], client=[%d]\n", credential->pid, client);

    su_context ctx = {
        .info = get_su_info(credential->uid),
        .req = su_request(),
        .pid = credential->pid
    };

    // Read su_request
    xxread(client, &ctx.req, sizeof(su_req_base));
    read_string(client, ctx.req.shell);
    read_string(client, ctx.req.command);

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
    log_cb.ex = exit;

    // ack
    write_int(client, 0);

    // Become session leader
    xsetsid();

    // Get pts_slave
    string pts_slave = read_string(client);

    // The FDs for each of the streams
    int infd  = recv_fd(client);
    int outfd = recv_fd(client);
    int errfd = recv_fd(client);

    if (!pts_slave.empty()) {
        LOGD("su: pts_slave=[%s]\n", pts_slave.data());
        // Check pts_slave file is owned by daemon_from_uid
        struct stat st;
        xstat(pts_slave.data(), &st);

        // If caller is not root, ensure the owner of pts_slave is the caller
        if(st.st_uid != ctx.info->uid && ctx.info->uid != 0)
            LOGE("su: Wrong permission of pts_slave\n");

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

    // Unleash all streams from SELinux hell
    setfilecon("/proc/self/fd/0", "u:object_r:" SEPOL_FILE_TYPE ":s0");
    setfilecon("/proc/self/fd/1", "u:object_r:" SEPOL_FILE_TYPE ":s0");
    setfilecon("/proc/self/fd/2", "u:object_r:" SEPOL_FILE_TYPE ":s0");

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
    snprintf(path, sizeof(path), "/proc/%d/cwd", ctx.pid);
    chdir(path);
    snprintf(path, sizeof(path), "/proc/%d/environ", ctx.pid);
    char buf[4096] = { 0 };
    int fd = xopen(path, O_RDONLY);
    read(fd, buf, sizeof(buf));
    close(fd);
    clearenv();
    for (size_t pos = 0; buf[pos];) {
        putenv(buf + pos);
        pos += strlen(buf + pos) + 1;
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
    exit(EXIT_FAILURE);
}
