/*
 * Copyright 2017 - 2025, John Wu (@topjohnwu)
 * Copyright 2015, Pierre-Hugues Husson <phh@phh.me>
 * Copyright 2010, Adam Shanks (@ChainsDD)
 * Copyright 2008, Zinx Verituse (@zinxv)
 */

#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <pwd.h>
#include <linux/securebits.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>

#include <algorithm>

#include <consts.hpp>
#include <base.hpp>
#include <flags.h>
#include <core.hpp>

using namespace std;

#define DEFAULT_SHELL "/system/bin/sh"

// Constants for atty
#define ATTY_IN    (1 << 0)
#define ATTY_OUT   (1 << 1)
#define ATTY_ERR   (1 << 2)

int quit_signals[] = { SIGALRM, SIGABRT, SIGHUP, SIGPIPE, SIGQUIT, SIGTERM, SIGINT, 0 };

[[noreturn]] static void usage(int status) {
    FILE *stream = (status == EXIT_SUCCESS) ? stdout : stderr;

    fprintf(stream,
    "MagiskSU\n\n"
    "Usage: su [options] [--] [user [argument...]]\n\n"
    "Options:\n"
    "  -s, --shell SHELL             Use SHELL instead of the default " DEFAULT_SHELL "\n"
    "  -i, --interactive             Force pseudo-terminal allocation\n"
    "  -g, --group GROUP             Specify the primary group\n"
    "  -G, --supp-group GROUP        Specify a supplementary group\n"
    "                                The first specified supplementary group is also used\n"
    "                                as a primary group if the option -g is not specified\n"
    "  -Z, --context CONTEXT         Change SELinux context\n"
    "  -t, --target PID              PID to take mount namespace from\n"
    "                                pid 0 means magisk global mount namespace\n"
    "  -d, --drop-cap                Drop all Linux capabilities\n"
    "  -m, -p,\n"
    "  --preserve-environment        Preserve the entire environment\n"
    "  -v, --version                 Display version number and exit\n"
    "  -V                            Display version code and exit\n"
    "  -h, --help                    Display this help message and exit\n\n"
    "--: Force stop options parsing, and also stop when an unknown option is found\n"
    "User: The user to switch to (default root), it can be name or uid\n"
    "Argument: Pass it to the shell as is\n\n");
    exit(status);
}

static void sighandler(int sig) {
    // Close all standard I/O to cause the pumps to exit
    // so we can continue and retrieve the exit code.
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Put back all the default handlers
    struct sigaction act{};
    act.sa_handler = SIG_DFL;
    for (int i = 0; quit_signals[i]; ++i) {
        sigaction(quit_signals[i], &act, nullptr);
    }
}

static void setup_sighandlers(void (*handler)(int)) {
    struct sigaction act{};
    act.sa_handler = handler;
    for (int i = 0; quit_signals[i]; ++i) {
        sigaction(quit_signals[i], &act, nullptr);
    }
}

int su_client_main(int argc, char *argv[]) {
    opterr = 0;
    option long_opts[] = {
            { "help",                   no_argument,        nullptr, 'h' },
            { "preserve-environment",   no_argument,        nullptr, 'p' },
            { "shell",                  required_argument,  nullptr, 's' },
            { "version",                no_argument,        nullptr, 'v' },
            { "context",                required_argument,  nullptr, 'Z' },
            { "mount-master",           no_argument,        nullptr, 'M' },
            { "target",                 required_argument,  nullptr, 't' },
            { "group",                  required_argument,  nullptr, 'g' },
            { "supp-group",             required_argument,  nullptr, 'G' },
            { "interactive",            no_argument,        nullptr, 'i' },
            { "drop-cap",               no_argument,        nullptr, 'd' },
            { nullptr, 0, nullptr, 0 },
    };

    auto req = SuRequest::New();

    for (int i = 0; i < argc; i++) {
        // Replace -cn and -z with -Z for backwards compatibility
        if (strcmp(argv[i], "-cn") == 0 || strcmp(argv[i], "-z") == 0)
            strcpy(argv[i], "-Z");
        // Replace -mm with -M for supporting getopt_long
        else if (strcmp(argv[i], "-mm") == 0)
            strcpy(argv[i], "-M");
    }

    bool interactive = false;
    string shell = DEFAULT_SHELL;

    int c;
    while ((c = getopt_long(argc, argv, "+:himpds:VvuZ:Mt:g:G:", long_opts, nullptr)) != -1) {
        switch (c) {
            case 'h':
                usage(EXIT_SUCCESS);
            case 'i':
                interactive = true;
                break;
            case 'm':
            case 'p':
                req.keep_env = true;
                break;
            case 'd':
                req.drop_cap = true;
                break;
            case 's':
                shell = optarg;
                break;
            case 'V':
                printf("%d\n", MAGISK_VER_CODE);
                exit(EXIT_SUCCESS);
            case 'v':
                printf("%s\n", MAGISK_VERSION ":MAGISKSU");
                exit(EXIT_SUCCESS);
            case 'Z':
                req.context = optarg;
                break;
            case 'M':
            case 't':
                if (req.target_pid != -1) {
                    fprintf(stderr, "Can't use -M and -t at the same time\n");
                    usage(EXIT_FAILURE);
                }
                if (optarg == nullptr) {
                    req.target_pid = 0;
                } else {
                    req.target_pid = parse_int(optarg);
                    if (*optarg == '-' || req.target_pid == -1) {
                        fprintf(stderr, "Invalid PID: %s\n", optarg);
                        usage(EXIT_FAILURE);
                    }
                }
                break;
            case 'g':
            case 'G': {
                if (int gid = parse_int(optarg); gid >= 0) {
                    if (c == 'g' && !req.gids.empty()) {
                        req.gids.push_back(req.gids[0]);
                        req.gids[0] = gid;
                    } else {
                        req.gids.push_back(gid);
                    }
                } else {
                    fprintf(stderr, "Invalid GID: %s\n", optarg);
                    usage(EXIT_FAILURE);
                }
                break;
            }
            case ':':
                fprintf(stderr, "option '%s' requires an argument\n", argv[optind - 1]);
                usage(2);
            case '?':
                optind--;
                goto end;
        }
    }

end:
    /* username or uid */
    if (optind < argc) {
        if (const passwd *pw = getpwnam(argv[optind])) {
            req.target_uid = pw->pw_uid;
            optind++;
        } else if (int uid = parse_int(argv[optind]); uid >= 0) {
            req.target_uid = uid;
            optind++;
        }
    }

    req.command.emplace_back(shell.c_str());
    if (optind < argc) {
        for (int i = optind; i < argc; ++i) {
            req.command.push_back(argv[i]);
        }
        optind = argc;
    }

    // Connect to client
    owned_fd fd = connect_daemon(RequestCode::SUPERUSER);

    // Send request
    req.write_to_fd(fd);

    // Wait for ack from daemon
    if (read_int(fd)) {
        // Fast fail
        fprintf(stderr, "%s\n", strerror(EACCES));
        return EACCES;
    }

    // Determine which one of our streams are attached to a TTY
    interactive |= req.command.size() == 1;
    int atty = 0;
    if (isatty(STDIN_FILENO) && interactive)  atty |= ATTY_IN;
    if (isatty(STDOUT_FILENO) && interactive) atty |= ATTY_OUT;
    if (isatty(STDERR_FILENO) && interactive) atty |= ATTY_ERR;

    // Send stdin
    send_fd(fd, (atty & ATTY_IN) ? -1 : STDIN_FILENO);
    // Send stdout
    send_fd(fd, (atty & ATTY_OUT) ? -1 : STDOUT_FILENO);
    // Send stderr
    send_fd(fd, (atty & ATTY_ERR) ? -1 : STDERR_FILENO);

    if (atty) {
        // We need a PTY. Get one.
        int ptmx = recv_fd(fd);
        setup_sighandlers(sighandler);
        // If stdin is not a tty, and if we pump to ptmx, our process may intercept the input to ptmx and
        // output to stdout, which cause the target process lost input.
        pump_tty(ptmx, atty & ATTY_IN);
    }

    // Get the exit code
    return read_int(fd);
}

static void drop_caps() {
    static auto last_valid_cap = []() {
        uint32_t cap = CAP_WAKE_ALARM;
        while (prctl(PR_CAPBSET_READ, cap) >= 0) {
            cap++;
        }
        return cap - 1;
    }();
    // Drop bounding set
    for (uint32_t cap = 0; cap <= last_valid_cap; cap++) {
        if (cap != CAP_SETUID) {
            prctl(PR_CAPBSET_DROP, cap);
        }
    }
    // Clean inheritable set
    __user_cap_header_struct header = {.version = _LINUX_CAPABILITY_VERSION_3};
    __user_cap_data_struct data[_LINUX_CAPABILITY_U32S_3] = {};
    if (capget(&header, &data[0]) == 0) {
        for (size_t i = 0; i < _LINUX_CAPABILITY_U32S_3; i++) {
            data[i].inheritable = 0;
        }
        capset(&header, &data[0]);
    }
    // All capabilities will be lost after exec
    prctl(PR_SET_SECUREBITS, SECBIT_NOROOT);
    // Except CAP_SETUID in bounding set, it is a marker for restricted process
}

static bool proc_is_restricted(pid_t pid) {
    char buf[32] = {};
    auto bnd = "CapBnd:"sv;
    uint32_t data[_LINUX_CAPABILITY_U32S_3] = {};
    ssprintf(buf, sizeof(buf), "/proc/%d/status", pid);
    owned_fd status_fd = xopen(buf, O_RDONLY | O_CLOEXEC);
    file_readline(status_fd, [&](Utf8CStr s) -> bool {
        string_view line = s;
        if (line.starts_with(bnd)) {
            auto p = line.begin();
            advance(p, bnd.size());
            while (isspace(*p)) advance(p, 1);
            line.remove_prefix(distance(line.begin(), p));
            for (int i = 0; i < _LINUX_CAPABILITY_U32S_3; i++) {
                auto cap = line.substr((_LINUX_CAPABILITY_U32S_3 - 1 - i) * 8, 8);
                data[i] = parse_uint32_hex(cap);
            }
            return false;
        }
        return true;
    });

    bool equal = true;
    for (int i = 0; i < _LINUX_CAPABILITY_U32S_3; i++) {
        if (i == CAP_TO_INDEX(CAP_SETUID)) {
            if (data[i] != CAP_TO_MASK(CAP_SETUID)) equal = false;
        } else {
            if (data[i] != 0) equal = false;
        }
    }
    return equal;
}

static void set_identity(int uid, const rust::Vec<gid_t> &groups) {
    gid_t gid;
    if (!groups.empty()) {
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

void exec_root_shell(int client, int pid, SuRequest &req, MntNsMode mode) {
    // Become session leader
    xsetsid();

    // The FDs for each of the streams
    int infd = recv_fd(client);
    int outfd = recv_fd(client);
    int errfd = recv_fd(client);
    int ptsfd = -1;

    // App need a PTY
    if (infd < 0 || outfd < 0 || errfd < 0) {
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
        ptsfd = xopen(pts_slave.data(), O_RDWR);
    }

    // Swap out stdin, stdout, stderr
    xdup2(infd < 0 ? ptsfd : infd, STDIN_FILENO);
    xdup2(outfd < 0 ? ptsfd : outfd, STDOUT_FILENO);
    xdup2(errfd < 0 ? ptsfd : errfd, STDERR_FILENO);

    close(infd);
    close(outfd);
    close(errfd);
    close(ptsfd);
    close(client);

    // Handle namespaces
    if (req.target_pid == -1)
        req.target_pid = pid;
    else if (req.target_pid == 0)
        mode = MntNsMode::Global;
    else if (mode == MntNsMode::Global)
        mode = MntNsMode::Requester;

    switch (mode) {
        case MntNsMode::Global:
            LOGD("su: use global namespace\n");
            break;
        case MntNsMode::Requester:
            LOGD("su: use namespace of pid=[%d]\n", req.target_pid);
            switch_mnt_ns(req.target_pid);
            break;
        case MntNsMode::Isolate:
            LOGD("su: use new isolated namespace\n");
            switch_mnt_ns(req.target_pid);
            xunshare(CLONE_NEWNS);
            xmount(nullptr, "/", nullptr, MS_PRIVATE | MS_REC, nullptr);
            break;
    }

    vector<const char *> argv;
    for (auto &str: req.command) {
        argv.push_back(str.c_str());
    }
    argv.push_back(nullptr);

    // Setup environment
    umask(022);
    char path[32];
    ssprintf(path, sizeof(path), "/proc/%d/cwd", pid);
    char cwd[4096];
    if (canonical_path(path, cwd, sizeof(cwd)) > 0)
        chdir(cwd);
    ssprintf(path, sizeof(path), "/proc/%d/environ", pid);
    auto env = full_read(path);
    clearenv();
    for (size_t pos = 0; pos < env.size(); ++pos) {
        putenv(env.data() + pos);
        pos = env.find_first_of('\0', pos);
        if (pos == std::string::npos)
            break;
    }
    if (!req.keep_env) {
        struct passwd *pw;
        pw = getpwuid(req.target_uid);
        if (pw) {
            setenv("HOME", pw->pw_dir, 1);
            setenv("USER", pw->pw_name, 1);
            setenv("LOGNAME", pw->pw_name, 1);
            setenv("SHELL", argv[0], 1);
        }
    }

    // Config privileges
    if (!req.context.empty()) {
        auto f = xopen_file("/proc/self/attr/exec", "we");
        if (f) fprintf(f.get(), "%s", req.context.c_str());
    }
    if (req.target_uid != AID_ROOT || req.drop_cap || proc_is_restricted(pid))
        drop_caps();
    if (req.target_uid != AID_ROOT || req.gids.size() > 0)
        set_identity(req.target_uid, req.gids);

    // Unblock all signals
    sigset_t block_set;
    sigemptyset(&block_set);
    sigprocmask(SIG_SETMASK, &block_set, nullptr);

    execvp(argv[0], const_cast<char* const*>(argv.data()));
    fprintf(stderr, "Cannot execute %s: %s\n", argv[0], strerror(errno));
    PLOGE("exec");
}
