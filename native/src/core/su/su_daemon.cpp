#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/mount.h>

#include <consts.hpp>
#include <base.hpp>
#include <selinux.hpp>

#include "su.hpp"
#include "pts.hpp"

using namespace std;

static pthread_mutex_t cache_lock = PTHREAD_MUTEX_INITIALIZER;
static shared_ptr<su_info> cached;

su_info::su_info(int uid) :
uid(uid), eval_uid(-1), cfg(DbSettings()), access(RootSettings()),
mgr_uid(-1), timestamp(0), _lock(PTHREAD_MUTEX_INITIALIZER) {}

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
    auto &daemon = MagiskD();
    daemon.get_db_settings(cfg);

    // Check multiuser settings
    switch (cfg.multiuser_mode) {
    case MultiuserMode::OwnerOnly:
        if (to_user_id(uid) != 0) {
            eval_uid = -1;
            access = SILENT_DENY;
        }
        break;
    case MultiuserMode::OwnerManaged:
        eval_uid = to_app_id(uid);
        break;
    case MultiuserMode::User:
    default:
        break;
    }

    if (eval_uid > 0) {
        if (!daemon.get_root_settings(eval_uid, access))
            return;
    }

    // We need to check our manager
    if (access.policy == SuPolicy::Query || access.log || access.notify) {
        mgr_uid = daemon.get_manager(to_user_id(eval_uid), &mgr_pkg, true);
    }
}

static shared_ptr<su_info> get_su_info(unsigned uid) {
    if (uid == AID_ROOT) {
        auto info = make_shared<su_info>(uid);
        info->access = SILENT_ALLOW;
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

    if (info->access.policy == SuPolicy::Query) {
        // Not cached, get data from database
        info->check_db();

        // If it's the manager, allow it silently
        if (to_app_id(info->uid) == to_app_id(info->mgr_uid)) {
            info->access = SILENT_ALLOW;
            return info;
        }

        // Check su access settings
        switch (info->cfg.root_access) {
            case RootAccess::Disabled:
                LOGW("Root access is disabled!\n");
                info->access = SILENT_DENY;
                break;
            case RootAccess::AdbOnly:
                if (info->uid != AID_SHELL) {
                    LOGW("Root access limited to ADB only!\n");
                    info->access = SILENT_DENY;
                }
                break;
            case RootAccess::AppsOnly:
                if (info->uid == AID_SHELL) {
                    LOGW("Root access is disabled for ADB!\n");
                    info->access = SILENT_DENY;
                }
                break;
            case RootAccess::AppsAndAdb:
            default:
                break;
        }

        if (info->access.policy != SuPolicy::Query)
            return info;

        // If still not determined, check if manager exists
        if (info->mgr_uid < 0) {
            info->access = SILENT_DENY;
            return info;
        }
    }
    return info;
}

// Set effective uid back to root, otherwise setres[ug]id will fail if uid isn't root
static void set_identity(uid_t uid, const std::vector<uid_t> &groups) {
    if (seteuid(0)) {
        PLOGE("seteuid (root)");
    }
    gid_t gid;
    if (groups.size() > 0) {
        if (setgroups(groups.size(), groups.data())) {
            PLOGE("setgroups");
        }
        gid = groups[0];
    } else {
        gid = uid;
    }
    if (setresgid(gid, gid, gid)) {
        PLOGE("setresgid (%u)", uid);
    }
    if (setresuid(uid, uid, uid)) {
        PLOGE("setresuid (%u)", uid);
    }
}

void su_daemon_handler(int client, const sock_cred *cred) {
    LOGD("su: request from uid=[%d], pid=[%d], client=[%d]\n", cred->uid, cred->pid, client);

    su_context ctx = {
        .info = get_su_info(cred->uid),
        .req = su_request(),
        .pid = cred->pid
    };

    // Read su_request
    if (xxread(client, &ctx.req, sizeof(su_req_base)) < 0
        || !read_string(client, ctx.req.shell)
        || !read_string(client, ctx.req.command)
        || !read_string(client, ctx.req.context)
        || !read_vector(client, ctx.req.gids)) {
        LOGW("su: remote process probably died, abort\n");
        ctx.info.reset();
        write_int(client, +SuPolicy::Deny);
        close(client);
        return;
    }

    // If still not determined, ask manager
    if (ctx.info->access.policy == SuPolicy::Query) {
        int fd = app_request(ctx);
        if (fd < 0) {
            ctx.info->access.policy = SuPolicy::Deny;
        } else {
            int ret = read_int_be(fd);
            ctx.info->access.policy = ret < 0 ? SuPolicy::Deny : static_cast<SuPolicy>(ret);
            close(fd);
        }
    }

    if (ctx.info->access.log)
        app_log(ctx);
    else if (ctx.info->access.notify)
        app_notify(ctx);

    // Fail fast
    if (ctx.info->access.policy == SuPolicy::Deny) {
        LOGW("su: request rejected (%u)\n", ctx.info->uid);
        ctx.info.reset();
        write_int(client, +SuPolicy::Deny);
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
        auto magiskpts = get_magisk_tmp() + "/"s SHELLPTS;
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
    if (ctx.req.target == -1)
        ctx.req.target = ctx.pid;
    else if (ctx.req.target == 0)
        ctx.info->cfg.mnt_ns = MntNsMode::Global;
    else if (ctx.info->cfg.mnt_ns == MntNsMode::Global)
        ctx.info->cfg.mnt_ns = MntNsMode::Requester;
    switch (ctx.info->cfg.mnt_ns) {
        case MntNsMode::Global:
            LOGD("su: use global namespace\n");
            break;
        case MntNsMode::Requester:
            LOGD("su: use namespace of pid=[%d]\n", ctx.req.target);
            if (switch_mnt_ns(ctx.req.target))
                LOGD("su: setns failed, fallback to global\n");
            break;
        case MntNsMode::Isolate:
            LOGD("su: use new isolated namespace\n");
            switch_mnt_ns(ctx.req.target);
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
    if (!ctx.req.context.empty()) {
        auto f = xopen_file("/proc/self/attr/exec", "we");
        if (f) fprintf(f.get(), "%s", ctx.req.context.data());
    }
    set_identity(ctx.req.uid, ctx.req.gids);
    execvp(ctx.req.shell.data(), (char **) argv);
    fprintf(stderr, "Cannot execute %s: %s\n", ctx.req.shell.data(), strerror(errno));
    PLOGE("exec");
}
