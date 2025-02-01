/*
 * Copyright 2017 - 2023, John Wu (@topjohnwu)
 * Copyright 2015, Pierre-Hugues Husson <phh@phh.me>
 * Copyright 2010, Adam Shanks (@ChainsDD)
 * Copyright 2008, Zinx Verituse (@zinxv)
 */

#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <pwd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <algorithm>

#include <consts.hpp>
#include <base.hpp>
#include <flags.h>
#include <core.hpp>

#include "pts.hpp"

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
    "Usage: su [options] [-] [user [argument...]]\n\n"
    "Options:\n"
    "  -c, --command COMMAND         Pass COMMAND to the invoked shell\n"
    "  -g, --group GROUP             Specify the primary group\n"
    "  -G, --supp-group GROUP        Specify a supplementary group.\n"
    "                                The first specified supplementary group is also used\n"
    "                                as a primary group if the option -g is not specified.\n"
    "  -Z, --context CONTEXT         Change SELinux context\n"
    "  -t, --target PID              PID to take mount namespace from\n"
    "  -h, --help                    Display this help message and exit\n"
    "  -, -l, --login                Pretend the shell to be a login shell\n"
    "  -m, -p,\n"
    "  --preserve-environment        Preserve the entire environment\n"
    "  -s, --shell SHELL             Use SHELL instead of the default " DEFAULT_SHELL "\n"
    "  -v, --version                 Display version number and exit\n"
    "  -V                            Display version code and exit\n"
    "  -mm, -M,\n"
    "  --mount-master                Force run in the global mount namespace\n\n");
    exit(status);
}

static void sighandler(int sig) {
    restore_stdin();

    // Assume we'll only be called before death
    // See note before sigaction() in set_stdin_raw()
    //
    // Now, close all standard I/O to cause the pumps
    // to exit so we can continue and retrieve the exit
    // code
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Put back all the default handlers
    struct sigaction act;

    memset(&act, 0, sizeof(act));
    act.sa_handler = SIG_DFL;
    for (int i = 0; quit_signals[i]; ++i) {
        sigaction(quit_signals[i], &act, nullptr);
    }
}

static void setup_sighandlers(void (*handler)(int)) {
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    for (int i = 0; quit_signals[i]; ++i) {
        sigaction(quit_signals[i], &act, nullptr);
    }
}

int su_client_main(int argc, char *argv[]) {
    int c;
    struct option long_opts[] = {
            { "command",                required_argument,  nullptr, 'c' },
            { "help",                   no_argument,        nullptr, 'h' },
            { "login",                  no_argument,        nullptr, 'l' },
            { "preserve-environment",   no_argument,        nullptr, 'p' },
            { "shell",                  required_argument,  nullptr, 's' },
            { "version",                no_argument,        nullptr, 'v' },
            { "context",                required_argument,  nullptr, 'Z' },
            { "mount-master",           no_argument,        nullptr, 'M' },
            { "target",                 required_argument,  nullptr, 't' },
            { "group",                  required_argument,  nullptr, 'g' },
            { "supp-group",             required_argument,  nullptr, 'G' },
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

    while ((c = getopt_long(argc, argv, "c:hlmps:VvuZ:Mt:g:G:", long_opts, nullptr)) != -1) {
        switch (c) {
            case 'c': {
                string command;
                for (int i = optind - 1; i < argc; ++i) {
                    if (!command.empty())
                        command += ' ';
                    command += argv[i];
                }
                req.command = command;
                optind = argc;
                break;
            }
            case 'h':
                usage(EXIT_SUCCESS);
            case 'l':
                req.login = true;
                break;
            case 'm':
            case 'p':
                req.keep_env = true;
                break;
            case 's':
                req.shell = optarg;
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
                vector<gid_t> gids;
                if (int gid = parse_int(optarg); gid >= 0) {
                    gids.insert(c == 'g' ? gids.begin() : gids.end(), gid);
                } else {
                    fprintf(stderr, "Invalid GID: %s\n", optarg);
                    usage(EXIT_FAILURE);
                }
                std::copy(gids.begin(), gids.end(), std::back_inserter(req.gids));
                break;
            }
            default:
                /* Bionic getopt_long doesn't terminate its error output by newline */
                fprintf(stderr, "\n");
                usage(2);
        }
    }

    if (optind < argc && strcmp(argv[optind], "-") == 0) {
        req.login = true;
        optind++;
    }
    /* username or uid */
    if (optind < argc) {
        struct passwd *pw;
        pw = getpwnam(argv[optind]);
        if (pw)
            req.target_uid = pw->pw_uid;
        else
            req.target_uid = parse_int(argv[optind]);
        optind++;
    }

    int ptmx, fd;

    // Connect to client
    fd = connect_daemon(+RequestCode::SUPERUSER);

    // Send request
    req.write_to_fd(fd);

    // Wait for ack from daemon
    if (read_int(fd)) {
        // Fast fail
        fprintf(stderr, "%s\n", strerror(EACCES));
        return EACCES;
    }

    // Determine which one of our streams are attached to a TTY
    int atty = 0;
    if (isatty(STDIN_FILENO))  atty |= ATTY_IN;
    if (isatty(STDOUT_FILENO)) atty |= ATTY_OUT;
    if (isatty(STDERR_FILENO)) atty |= ATTY_ERR;

    // Send stdin
    send_fd(fd, (atty & ATTY_IN) ? -1 : STDIN_FILENO);
    // Send stdout
    send_fd(fd, (atty & ATTY_OUT) ? -1 : STDOUT_FILENO);
    // Send stderr
    send_fd(fd, (atty & ATTY_ERR) ? -1 : STDERR_FILENO);

    if (atty) {
        // We need a PTY. Get one.
        write_int(fd, 1);
        ptmx = recv_fd(fd);
    } else {
        write_int(fd, 0);
    }

    if (atty) {
        setup_sighandlers(sighandler);
        watch_sigwinch_async(STDOUT_FILENO, ptmx);
        pump_stdin_async(ptmx);
        pump_stdout_blocking(ptmx);
    }

    // Get the exit code
    int code = read_int(fd);
    close(fd);

    return code;
}

// Set effective uid back to root, otherwise setres[ug]id will fail if uid isn't root
static void set_identity(int uid, const rust::Vec<gid_t> &groups) {
    if (seteuid(0)) {
        PLOGE("seteuid (root)");
    }
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

    const char *argv[4] = { nullptr };

    argv[0] = req.login ? "-" : req.shell.c_str();

    if (!req.command.empty()) {
        argv[1] = "-c";
        argv[2] = req.command.c_str();
    }

    // Setup environment
    umask(022);
    char path[32];
    ssprintf(path, sizeof(path), "/proc/%d/cwd", pid);
    char cwd[4096];
    if (realpath(path, cwd, sizeof(cwd)) > 0)
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
            setenv("SHELL", req.shell.c_str(), 1);
        }
    }

    // Unblock all signals
    sigset_t block_set;
    sigemptyset(&block_set);
    sigprocmask(SIG_SETMASK, &block_set, nullptr);
    if (!req.context.empty()) {
        auto f = xopen_file("/proc/self/attr/exec", "we");
        if (f) fprintf(f.get(), "%s", req.context.c_str());
    }
    set_identity(req.target_uid, req.gids);
    execvp(req.shell.c_str(), (char **) argv);
    fprintf(stderr, "Cannot execute %s: %s\n", req.shell.c_str(), strerror(errno));
    PLOGE("exec");
}
