/*
 * Copyright 2017 - 2021, John Wu (@topjohnwu)
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

#include <magisk.hpp>
#include <daemon.hpp>
#include <utils.hpp>
#include <flags.hpp>

#include "su.hpp"
#include "pts.hpp"

int quit_signals[] = { SIGALRM, SIGABRT, SIGHUP, SIGPIPE, SIGQUIT, SIGTERM, SIGINT, 0 };

static void usage(int status) {
    FILE *stream = (status == EXIT_SUCCESS) ? stdout : stderr;

    fprintf(stream,
    "MagiskSU\n\n"
    "Usage: su [options] [-] [user [argument...]]\n\n"
    "Options:\n"
    "  -c, --command COMMAND         pass COMMAND to the invoked shell\n"
    "  -h, --help                    display this help message and exit\n"
    "  -, -l, --login                pretend the shell to be a login shell\n"
    "  -m, -p,\n"
    "  --preserve-environment        preserve the entire environment\n"
    "  -s, --shell SHELL             use SHELL instead of the default " DEFAULT_SHELL "\n"
    "  -v, --version                 display version number and exit\n"
    "  -V                            display version code and exit\n"
    "  -mm, -M,\n"
    "  --mount-master                force run in the global mount namespace\n");
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
            { "context",                required_argument,  nullptr, 'z' },
            { "mount-master",           no_argument,        nullptr, 'M' },
            { nullptr, 0, nullptr, 0 },
    };

    su_request su_req;

    for (int i = 0; i < argc; i++) {
        // Replace -cn with -z, -mm with -M for supporting getopt_long
        if (strcmp(argv[i], "-cn") == 0)
            strcpy(argv[i], "-z");
        else if (strcmp(argv[i], "-mm") == 0)
            strcpy(argv[i], "-M");
    }

    while ((c = getopt_long(argc, argv, "c:hlmps:Vvuz:M", long_opts, nullptr)) != -1) {
        switch (c) {
            case 'c':
                for (int i = optind - 1; i < argc; ++i) {
                    if (!su_req.command.empty())
                        su_req.command += ' ';
                    su_req.command += argv[i];
                }
                optind = argc;
                break;
            case 'h':
                usage(EXIT_SUCCESS);
                break;
            case 'l':
                su_req.login = true;
                break;
            case 'm':
            case 'p':
                su_req.keepenv = true;
                break;
            case 's':
                su_req.shell = optarg;
                break;
            case 'V':
                printf("%d\n", MAGISK_VER_CODE);
                exit(EXIT_SUCCESS);
            case 'v':
                printf("%s\n", MAGISK_VERSION ":MAGISKSU");
                exit(EXIT_SUCCESS);
            case 'z':
                // Do nothing, placed here for legacy support :)
                break;
            case 'M':
                su_req.mount_master = true;
                break;
            default:
                /* Bionic getopt_long doesn't terminate its error output by newline */
                fprintf(stderr, "\n");
                usage(2);
        }
    }

    if (optind < argc && strcmp(argv[optind], "-") == 0) {
        su_req.login = true;
        optind++;
    }
    /* username or uid */
    if (optind < argc) {
        struct passwd *pw;
        pw = getpwnam(argv[optind]);
        if (pw)
            su_req.uid = pw->pw_uid;
        else
            su_req.uid = parse_int(argv[optind]);
        optind++;
    }

    char pts_slave[PATH_MAX];
    int ptmx, fd;

    // Connect to client
    fd = connect_daemon();

    // Tell the daemon we are su
    write_int(fd, SUPERUSER);

    // Send su_request
    xwrite(fd, &su_req, sizeof(su_req_base));
    write_string(fd, su_req.shell);
    write_string(fd, su_req.command);

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

    if (atty) {
        // We need a PTY. Get one.
        ptmx = pts_open(pts_slave, sizeof(pts_slave));
    } else {
        pts_slave[0] = '\0';
    }

    // Send pts_slave
    write_string(fd, pts_slave);

    // Send stdin
    send_fd(fd, (atty & ATTY_IN) ? -1 : STDIN_FILENO);
    // Send stdout
    send_fd(fd, (atty & ATTY_OUT) ? -1 : STDOUT_FILENO);
    // Send stderr
    send_fd(fd, (atty & ATTY_ERR) ? -1 : STDERR_FILENO);

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
