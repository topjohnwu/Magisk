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

static void magisk_logging();

static void daemon_entry(int ppid) {
    magisk_logging();

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

    LOGI(NAME_WITH_VER(Magisk) " daemon started\n");

    // Make sure ppid is not in acct
    char src[64], dest[64];
    sprintf(src, "/acct/uid_0/pid_%d", ppid);
    if (access(src, F_OK) == 0) {
        sprintf(dest, "/acct/uid_0/pid_%d", getpid());
        rename(src, dest);
    }
    sprintf(src, "/sys/fs/cgroup/uid_0/pid_%d", ppid);
    if (access(src, F_OK) == 0) {
        sprintf(dest, "/sys/fs/cgroup/uid_0/pid_%d", getpid());
        rename(src, dest);
    }

    // Get self stat
    xreadlink("/proc/self/exe", src, sizeof(src));
    MAGISKTMP = dirname(src);
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

    // Change process name
    set_nice_name("magiskd");

    // Block all signals
    sigset_t block_set;
    sigfillset(&block_set);
    pthread_sigmask(SIG_SETMASK, &block_set, nullptr);

    // Loop forever to listen for requests
    for (;;) {
        int client = xaccept4(fd, nullptr, nullptr, SOCK_CLOEXEC);
        handle_request(client);
    }
}

int connect_daemon(bool create) {
    sockaddr_un sun;
    socklen_t len = setup_sockaddr(&sun, MAIN_SOCKET);
    int fd = xsocket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (connect(fd, (struct sockaddr*) &sun, len)) {
        if (!create || getuid() != UID_ROOT || getgid() != UID_ROOT) {
            LOGE("No daemon is currently running!\n");
            exit(1);
        }

        int ppid = getpid();
        LOGD("client: launching new main daemon process\n");
        if (fork_dont_care() == 0) {
            close(fd);
            daemon_entry(ppid);
        }

        while (connect(fd, (struct sockaddr*) &sun, len))
            usleep(10000);
    }
    return fd;
}

#define TMP_LOGFILE "/dev/.magisk.log"

namespace {
class fd_holder {
public:
    explicit fd_holder(int fd) : fd(fd) {}
    ~fd_holder() { close(fd); }
    FILE *get_fp() { auto fp = fdopen(fd, "a"); setbuf(fp, nullptr); return fp; }
private:
    int fd;
};
}

static shared_ptr<fd_holder> g_log_file;

void setup_logfile(bool reset) {
    static atomic_flag has_setup = ATOMIC_FLAG_INIT;
    if (has_setup.test_and_set(memory_order_relaxed))
        return;

    if (reset)
        rename(LOGFILE, LOGFILE ".bak");

    g_log_file.reset();

    int fd = open(LOGFILE, O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC, 0644);
    if (fd < 0)
        return;

    if (int old = open(TMP_LOGFILE, O_RDONLY | O_CLOEXEC); old >= 0) {
        char buf[4096];
        int sz;
        while ((sz = xread(old, buf, sizeof(buf))) > 0) {
            xwrite(fd, buf, sz);
        }
        close(old);
    }
    unlink(TMP_LOGFILE);
    g_log_file = make_shared<fd_holder>(fd);
}

static int magisk_log(int prio, const char *fmt, va_list ap) {
    // Block all signals temporarily
    sigset_t block_set, old_set;
    sigfillset(&block_set);
    pthread_sigmask(SIG_SETMASK, &block_set, &old_set);
    run_finally fin([set = &old_set] { pthread_sigmask(SIG_SETMASK, set, nullptr); });

    va_list args;
    va_copy(args, ap);

    // Log to logcat
    __android_log_vprint(prio, "Magisk", fmt, ap);

    // Create local copy to prevent race condition
    auto log_file = g_log_file;
    if (!log_file)
        return 0;

    char buf[4096];
    char type;
    switch (prio) {
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
    timeval tv;
    tm tm;
    gettimeofday(&tv, nullptr);
    localtime_r(&tv.tv_sec, &tm);
    size_t len = strftime(buf, sizeof(buf), "%m-%d %T", &tm);
    int ms = tv.tv_usec / 1000;
    len += sprintf(buf + len, ".%03d %*d %*d %c : ", ms, 5, getpid(), 5, gettid(), type);
    strcpy(buf + len, fmt);
    return vfprintf(log_file->get_fp(), buf, args);
}

#define mlog(prio) [](auto fmt, auto ap){ return magisk_log(ANDROID_LOG_##prio, fmt, ap); }
static void magisk_logging() {
    if (int fd = open(TMP_LOGFILE, O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC, 0644); fd >= 0) {
        g_log_file = make_shared<fd_holder>(fd);
    }

    log_cb.d = mlog(DEBUG);
    log_cb.i = mlog(INFO);
    log_cb.w = mlog(WARN);
    log_cb.e = mlog(ERROR);
    log_cb.ex = nop_ex;
}
