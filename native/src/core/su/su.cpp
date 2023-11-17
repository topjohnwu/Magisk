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

#include <consts.hpp>
#include <base.hpp>
#include <flags.h>

#include "su.hpp"
#include "pts.hpp"

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

    su_request su_req;

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
            case 'Z':
                su_req.context = optarg;
                break;
            case 'M':
            case 't':
                if (su_req.target != -1) {
                    fprintf(stderr, "Can't use -M and -t at the same time\n");
                    usage(EXIT_FAILURE);
                }
                if (optarg == nullptr) {
                    su_req.target = 0;
                } else {
                    su_req.target = parse_int(optarg);
                    if (*optarg == '-' || su_req.target == -1) {
                        fprintf(stderr, "Invalid PID: %s\n", optarg);
                        usage(EXIT_FAILURE);
                    }
                }
                break;
            case 'g':
            case 'G':
                if (int gid = parse_int(optarg); gid >= 0) {
                    su_req.gids.insert(c == 'g' ? su_req.gids.begin() : su_req.gids.end(), gid);
                } else {
                    fprintf(stderr, "Invalid GID: %s\n", optarg);
                    usage(EXIT_FAILURE);
                }
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

    int ptmx, fd;

    // Connect to client
    fd = connect_daemon(+RequestCode::SUPERUSER);

    // Send su_request
    xwrite(fd, &su_req, sizeof(su_req_base));
    write_string(fd, su_req.shell);
    write_string(fd, su_req.command);
    write_string(fd, su_req.context);
    write_vector(fd, su_req.gids);

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
