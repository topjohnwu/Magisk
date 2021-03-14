#include <fcntl.h>
#include <pthread.h>
#include <csignal>
#include <libgen.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <android/log.h>

#include <magisk.hpp>
#include <utils.hpp>
#include <daemon.hpp>
#include <selinux.hpp>
#include <db.hpp>
#include <resetprop.hpp>
#include <flags.hpp>
#include <stream.hpp>

#include "core.hpp"

using namespace std;

int SDK_INT = -1;
string MAGISKTMP;

bool RECOVERY_MODE = false;
int DAEMON_STATE = STATE_NONE;

static struct stat self_st;

static bool verify_client(pid_t pid) {
    // Verify caller is the same as server
    char path[32];
    sprintf(path, "/proc/%d/exe", pid);
    struct stat st;
    return !(stat(path, &st) || st.st_dev != self_st.st_dev || st.st_ino != self_st.st_ino);
}

static bool check_zygote(pid_t pid) {
    char buf[32];
    sprintf(buf, "/proc/%d/attr/current", pid);
    auto fp = open_file(buf, "r");
    if (!fp)
        return false;
    fscanf(fp.get(), "%s", buf);
    return buf == "u:r:zygote:s0"sv;
}

static void request_handler(int client, int req_code, ucred cred) {
    switch (req_code) {
    case MAGISKHIDE:
        magiskhide_handler(client, &cred);
        break;
    case SUPERUSER:
        su_daemon_handler(client, &cred);
        break;
    case POST_FS_DATA:
        post_fs_data(client);
        break;
    case LATE_START:
        late_start(client);
        break;
    case BOOT_COMPLETE:
        boot_complete(client);
        break;
    case SQLITE_CMD:
        exec_sql(client);
        break;
    case REMOVE_MODULES:
        remove_modules();
        write_int(client, 0);
        close(client);
        reboot();
        break;
    default:
        close(client);
        break;
    }
}

static void handle_request(int client) {
    int req_code;

    // Verify client credentials
    ucred cred;
    get_client_cred(client, &cred);

    bool is_root = cred.uid == 0;
    bool is_zygote = check_zygote(cred.pid);
    bool is_client = verify_client(cred.pid);

    if (!is_root && !is_zygote && !is_client)
        goto shortcut;

    req_code = read_int(client);
    if (req_code < 0 || req_code >= DAEMON_CODE_END)
        goto shortcut;

    // Check client permissions
    switch (req_code) {
    case POST_FS_DATA:
    case LATE_START:
    case BOOT_COMPLETE:
    case SQLITE_CMD:
    case GET_PATH:
        if (!is_root) {
            write_int(client, ROOT_REQUIRED);
            goto shortcut;
        }
        break;
    case REMOVE_MODULES:
        if (cred.uid != UID_SHELL && cred.uid != UID_ROOT) {
            write_int(client, 1);
            goto shortcut;
        }
        break;
    case MAGISKHIDE:  // accept hide request from zygote
        if (!is_root && !is_zygote) {
            write_int(client, ROOT_REQUIRED);
            goto shortcut;
        }
        break;
    }

    // Simple requests
    switch (req_code) {
    case CHECK_VERSION:
        write_string(client, MAGISK_VERSION ":MAGISK");
        goto shortcut;
    case CHECK_VERSION_CODE:
        write_int(client, MAGISK_VER_CODE);
        goto shortcut;
    case GET_PATH:
        write_string(client, MAGISKTMP.data());
        goto shortcut;
    case START_DAEMON:
        setup_logfile(true);
        goto shortcut;
    }

    // Create new thread to handle complex requests
    new_daemon_thread([=] { return request_handler(client, req_code, cred); });
    return;

shortcut:
    close(client);
}

static int switch_cgroup(const char *cgroup, int pid) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%s/cgroup.procs", cgroup);
    int fd = open(buf, O_WRONLY | O_APPEND | O_CLOEXEC);
    if (fd == -1)
        return -1;
    snprintf(buf, sizeof(buf), "%d\n", pid);
    if (xwrite(fd, buf, strlen(buf)) == -1) {
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

static void magisk_logging();
static void start_log_daemon();

[[noreturn]] static void daemon_entry() {
    magisk_logging();

    // Block all signals
    sigset_t block_set;
    sigfillset(&block_set);
    pthread_sigmask(SIG_SETMASK, &block_set, nullptr);

    // Change process name
    set_nice_name("magiskd");

    int fd = xopen("/dev/null", O_WRONLY);
    xdup2(fd, STDOUT_FILENO);
    xdup2(fd, STDERR_FILENO);
    if (fd > STDERR_FILENO)
        close(fd);
    fd = xopen("/dev/zero", O_RDONLY);
    xdup2(fd, STDIN_FILENO);
    if (fd > STDERR_FILENO)
        close(fd);

    setsid();
    setcon("u:r:" SEPOL_PROC_DOMAIN ":s0");

    start_log_daemon();

    LOGI(NAME_WITH_VER(Magisk) " daemon started\n");

    // Escape from cgroup
    int pid = getpid();
    if (switch_cgroup("/acct", pid) && switch_cgroup("/sys/fs/cgroup", pid))
        LOGW("Can't switch cgroup\n");

    // Get self stat
    char buf[64];
    xreadlink("/proc/self/exe", buf, sizeof(buf));
    MAGISKTMP = dirname(buf);
    xstat("/proc/self/exe", &self_st);

    // Get API level
    parse_prop_file("/system/build.prop", [](auto key, auto val) -> bool {
        if (key == "ro.build.version.sdk") {
            SDK_INT = parse_int(val);
            return false;
        }
        return true;
    });
    if (SDK_INT < 0) {
        // In case some devices do not store this info in build.prop, fallback to getprop
        auto sdk = getprop("ro.build.version.sdk");
        if (!sdk.empty()) {
            SDK_INT = parse_int(sdk);
        }
    }
    LOGI("* Device API level: %d\n", SDK_INT);

    restore_tmpcon();

    // SAR cleanups
    auto mount_list = MAGISKTMP + "/" ROOTMNT;
    if (access(mount_list.data(), F_OK) == 0) {
        file_readline(true, mount_list.data(), [](string_view line) -> bool {
            umount2(line.data(), MNT_DETACH);
            return true;
        });
    }
    unlink("/dev/.se");

    // Load config status
    auto config = MAGISKTMP + "/" INTLROOT "/config";
    parse_prop_file(config.data(), [](auto key, auto val) -> bool {
        if (key == "RECOVERYMODE" && val == "true")
            RECOVERY_MODE = true;
        return true;
    });

    struct sockaddr_un sun;
    socklen_t len = setup_sockaddr(&sun, MAIN_SOCKET);
    fd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (xbind(fd, (struct sockaddr*) &sun, len))
        exit(1);
    xlisten(fd, 10);

    // Loop forever to listen for requests
    for (;;) {
        int client = xaccept4(fd, nullptr, nullptr, SOCK_CLOEXEC);
        handle_request(client);
    }
}

int connect_daemon(bool create) {
    sockaddr_un sun;
    socklen_t len = setup_sockaddr(&sun, MAIN_SOCKET);
    int fd = xsocket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (connect(fd, (struct sockaddr*) &sun, len)) {
        if (!create || getuid() != UID_ROOT || getgid() != UID_ROOT) {
            LOGE("No daemon is currently running!\n");
            exit(1);
        }

        if (fork_dont_care() == 0) {
            close(fd);
            daemon_entry();
        }

        while (connect(fd, (struct sockaddr*) &sun, len))
            usleep(10000);
    }
    return fd;
}

struct log_meta {
    int prio;
    int len;
    int pid;
    int tid;
};

static atomic<int> log_sockfd = -1;

void setup_logfile(bool reset) {
    if (log_sockfd < 0)
        return;

    msghdr msg{};
    iovec iov{};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    log_meta meta = {
        .prio = -1,
        .len = reset
    };

    iov.iov_base = &meta;
    iov.iov_len = sizeof(meta);
    sendmsg(log_sockfd, &msg, MSG_NOSIGNAL);
}

static void logfile_writer(int sockfd) {
    run_finally close_socket([=] {
        // Close up all logging sockets when thread dies
        close(sockfd);
        close(log_sockfd.exchange(-1));
    });

    char *log_buf;
    size_t buf_len;
    stream *log_strm = new byte_stream(log_buf, buf_len);

    msghdr msg{};
    iovec iov{};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    log_meta meta{};
    char buf[4096];

    for (;;) {
        // Read meta data
        iov.iov_base = &meta;
        iov.iov_len = sizeof(meta);
        if (recvmsg(sockfd, &msg, 0) <= 0)
            return;

        if (meta.prio < 0 && buf_len >= 0) {
            run_finally free_buf([&] {
                free(log_buf);
                log_buf = nullptr;
                buf_len = -1;
            });

            if (meta.len)
                rename(LOGFILE, LOGFILE ".bak");

            int fd = open(LOGFILE, O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC, 0644);
            if (fd < 0)
                return;
            if (log_buf)
                write(fd, log_buf, buf_len);

            delete log_strm;
            log_strm = new fd_stream(fd);
            continue;
        }

        timeval tv;
        tm tm;
        gettimeofday(&tv, nullptr);
        localtime_r(&tv.tv_sec, &tm);

        // Format detailed info
        char type;
        switch (meta.prio) {
            case ANDROID_LOG_DEBUG:
                type = 'D';
                break;
            case ANDROID_LOG_INFO:
                type = 'I';
                break;
            case ANDROID_LOG_WARN:
                type = 'W';
                break;
            default:
                type = 'E';
                break;
        }
        size_t off = strftime(buf, sizeof(buf), "%m-%d %T", &tm);
        int ms = tv.tv_usec / 1000;
        off += snprintf(buf + off, sizeof(buf) - off,
                ".%03d %5d %5d %c : ", ms, meta.pid, meta.tid, type);

        // Read log msg
        iov.iov_base = buf + off;
        iov.iov_len = meta.len;
        if (recvmsg(sockfd, &msg, 0) <= 0)
            return;
        log_strm->write(buf, off + meta.len);
    }
}

static int magisk_log(int prio, const char *fmt, va_list ap) {
    char buf[4000];
    int len = vsnprintf(buf, sizeof(buf), fmt, ap) + 1;

    if (log_sockfd >= 0) {
        log_meta meta = {
            .prio = prio,
            .len = len,
            .pid = getpid(),
            .tid = gettid()
        };

        msghdr msg{};
        iovec iov[2];
        msg.msg_iov = iov;
        msg.msg_iovlen = 2;

        iov[0].iov_base = &meta;
        iov[0].iov_len = sizeof(meta);
        iov[1].iov_base = buf;
        iov[1].iov_len = len;

        if (sendmsg(log_sockfd, &msg, MSG_NOSIGNAL) < 0) {
            // Stop trying to write to file
            close(log_sockfd.exchange(-1));
        }
    }
    __android_log_write(prio, "Magisk", buf);

    return len - 1;
}

static void start_log_daemon() {
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, fds) == 0) {
        log_sockfd = fds[0];
        new_daemon_thread([=] { logfile_writer(fds[1]); });
    }
}

#define mlog(prio) [](auto fmt, auto ap){ return magisk_log(ANDROID_LOG_##prio, fmt, ap); }
static void magisk_logging() {
    log_cb.d = mlog(DEBUG);
    log_cb.i = mlog(INFO);
    log_cb.w = mlog(WARN);
    log_cb.e = mlog(ERROR);
    log_cb.ex = nop_ex;
}
