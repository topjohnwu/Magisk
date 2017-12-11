/*
 * Copyright 2017, John Wu (@topjohnwu)
 * Copyright 2015, Pierre-Hugues Husson <phh@phh.me>
 * Copyright 2010, Adam Shanks (@ChainsDD)
 * Copyright 2008, Zinx Verituse (@zinxv)
 */

/* su.c - The main function running in the daemon
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <pwd.h>
#include <errno.h>
#include <signal.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/auxv.h>
#include <selinux/selinux.h>

#include "magisk.h"
#include "utils.h"
#include "su.h"

struct su_context *su_ctx;

static void usage(int status) {
	FILE *stream = (status == EXIT_SUCCESS) ? stdout : stderr;

	fprintf(stream,
	"MagiskSU v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ")\n\n"
	"Usage: su [options] [--] [-] [LOGIN] [--] [args...]\n\n"
	"Options:\n"
	"  -c, --command COMMAND         pass COMMAND to the invoked shell\n"
	"  -h, --help                    display this help message and exit\n"
	"  -, -l, --login                pretend the shell to be a login shell\n"
	"  -m, -p,\n"
	"  --preserve-environment        do not change environment variables\n"
	"  -s, --shell SHELL             use SHELL instead of the default " DEFAULT_SHELL "\n"
	"  -u                            display the multiuser mode and exit\n"
	"  -v, --version                 display version number and exit\n"
	"  -V                            display version code and exit,\n"
	"                                this is used almost exclusively by Superuser.apk\n"
	"  -mm, -M,\n"
	"  --mount-master                run in the global mount namespace,\n"
	"                                use if you need to publicly apply mounts\n");
	exit2(status);
}

static char *concat_commands(int argc, char *argv[]) {
	char command[ARG_MAX];
	command[0] = '\0';
	for (int i = optind - 1; i < argc; ++i) {
		if (strlen(command))
			sprintf(command, "%s %s", command, argv[i]);
		else
			sprintf(command, "%s", argv[i]);
	}
	return strdup(command);
}

static void populate_environment(const struct su_context *ctx) {
	struct passwd *pw;

	if (ctx->to.keepenv)
		return;

	pw = getpwuid(ctx->to.uid);
	if (pw) {
		setenv("HOME", pw->pw_dir, 1);
		if (ctx->to.shell)
			setenv("SHELL", ctx->to.shell, 1);
		else
			setenv("SHELL", DEFAULT_SHELL, 1);
		if (ctx->to.login || ctx->to.uid) {
			setenv("USER", pw->pw_name, 1);
			setenv("LOGNAME", pw->pw_name, 1);
		}
	}
}

void set_identity(unsigned uid) {
	/*
	 * Set effective uid back to root, otherwise setres[ug]id will fail
	 * if uid isn't root.
	 */
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

static __attribute__ ((noreturn)) void allow() {
	char *arg0;

	umask(su_ctx->umask);

	if (su_ctx->notify)
		app_send_result(su_ctx, ALLOW);

	char *binary = su_ctx->to.shell;
	if (su_ctx->to.command) {
		su_ctx->to.argv[--su_ctx->to.argc] = su_ctx->to.command;
		su_ctx->to.argv[--su_ctx->to.argc] = "-c";
	}

	arg0 = strrchr(binary, '/');
	arg0 = arg0 ? (arg0 + 1) : binary;
	if (su_ctx->to.login) {
		int s = strlen(arg0) + 2;
		char *p = xmalloc(s);
		*p = '-';
		strcpy(p + 1, arg0);
		arg0 = p;
	}

	populate_environment(su_ctx);
	set_identity(su_ctx->to.uid);

	setexeccon("u:r:su:s0");

	su_ctx->to.argv[--su_ctx->to.argc] = arg0;
	execvp(binary, su_ctx->to.argv + su_ctx->to.argc);
	fprintf(stderr, "Cannot execute %s: %s\n", binary, strerror(errno));
	PLOGE("exec");
	exit(EXIT_FAILURE);
}

static __attribute__ ((noreturn)) void deny() {
	if (su_ctx->notify)
		app_send_result(su_ctx, DENY);

	LOGW("su: request rejected (%u->%u)", su_ctx->info->uid, su_ctx->to.uid);
	fprintf(stderr, "%s\n", strerror(EACCES));
	exit(EXIT_FAILURE);
}

static void socket_cleanup() {
	if (su_ctx && su_ctx->sock_path[0]) {
		unlink(su_ctx->sock_path);
		su_ctx->sock_path[0] = 0;
	}
}

static void cleanup_signal(int sig) {
	socket_cleanup();
	exit2(EXIT_FAILURE);
}

__attribute__ ((noreturn)) void exit2(int status) {
	// Handle the pipe, or the daemon will get stuck
	if (su_ctx->info->policy == QUERY) {
		xwrite(su_ctx->pipefd[1], &su_ctx->info->policy, sizeof(su_ctx->info->policy));
		close(su_ctx->pipefd[0]);
		close(su_ctx->pipefd[1]);
	}
	exit(status);
}

int su_daemon_main(int argc, char **argv) {
	// Sanitize all secure environment variables (from linker_environ.c in AOSP linker).
	/* The same list than GLibc at this point */
	static const char* const unsec_vars[] = {
		"GCONV_PATH",
		"GETCONF_DIR",
		"HOSTALIASES",
		"LD_AUDIT",
		"LD_DEBUG",
		"LD_DEBUG_OUTPUT",
		"LD_DYNAMIC_WEAK",
		"LD_LIBRARY_PATH",
		"LD_ORIGIN_PATH",
		"LD_PRELOAD",
		"LD_PROFILE",
		"LD_SHOW_AUXV",
		"LD_USE_LOAD_BIAS",
		"LOCALDOMAIN",
		"LOCPATH",
		"MALLOC_TRACE",
		"MALLOC_CHECK_",
		"NIS_PATH",
		"NLSPATH",
		"RESOLV_HOST_CONF",
		"RES_OPTIONS",
		"TMPDIR",
		"TZDIR",
		"LD_AOUT_LIBRARY_PATH",
		"LD_AOUT_PRELOAD",
		// not listed in linker, used due to system() call
		"IFS",
	};
	if(getauxval(AT_SECURE)) {
		const char* const* cp   = unsec_vars;
		const char* const* endp = cp + sizeof(unsec_vars)/sizeof(unsec_vars[0]);
		while (cp < endp) {
			unsetenv(*cp);
			cp++;
		}
	}

	int c, socket_serv_fd, fd;
	char result[64];
	struct option long_opts[] = {
		{ "command",                required_argument,  NULL, 'c' },
		{ "help",                   no_argument,        NULL, 'h' },
		{ "login",                  no_argument,        NULL, 'l' },
		{ "preserve-environment",   no_argument,        NULL, 'p' },
		{ "shell",                  required_argument,  NULL, 's' },
		{ "version",                no_argument,        NULL, 'v' },
		{ "context",                required_argument,  NULL, 'z' },
		{ "mount-master",           no_argument,        NULL, 'M' },
		{ NULL, 0, NULL, 0 },
	};

	while ((c = getopt_long(argc, argv, "c:hlmps:Vvuz:M", long_opts, NULL)) != -1) {
		switch (c) {
		case 'c':
			su_ctx->to.command = concat_commands(argc, argv);
			optind = argc;
			su_ctx->notify = 1;
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		case 'l':
			su_ctx->to.login = 1;
			break;
		case 'm':
		case 'p':
			su_ctx->to.keepenv = 1;
			break;
		case 's':
			su_ctx->to.shell = optarg;
			break;
		case 'V':
			printf("%d\n", MAGISK_VER_CODE);
			exit2(EXIT_SUCCESS);
		case 'v':
			printf("%s\n", MAGISKSU_VER_STR);
			exit2(EXIT_SUCCESS);
		case 'u':
			switch (su_ctx->info->multiuser_mode) {
			case MULTIUSER_MODE_USER:
				printf("Owner only: Only owner has root access\n");
				break;
			case MULTIUSER_MODE_OWNER_MANAGED:
				printf("Owner managed: Only owner can manage root access and receive request prompts\n");
				break;
			case MULTIUSER_MODE_OWNER_ONLY:
				printf("User independent: Each user has its own separate root rules\n");
				break;
			}
			exit2(EXIT_SUCCESS);
		case 'z':
			// Do nothing, placed here for legacy support :)
			break;
		case 'M':
			su_ctx->info->mnt_ns = NAMESPACE_MODE_GLOBAL;
			break;
		default:
			/* Bionic getopt_long doesn't terminate its error output by newline */
			fprintf(stderr, "\n");
			usage(2);
		}
	}

	su_ctx->to.argc = argc;
	su_ctx->to.argv = argv;

	if (optind < argc && !strcmp(argv[optind], "-")) {
		su_ctx->to.login = 1;
		optind++;
	}
	/* username or uid */
	if (optind < argc && strcmp(argv[optind], "--")) {
		struct passwd *pw;
		pw = getpwnam(argv[optind]);
		if (!pw) {
			char *endptr;

			/* It seems we shouldn't do this at all */
			errno = 0;
			su_ctx->to.uid = strtoul(argv[optind], &endptr, 10);
			if (errno || *endptr) {
				LOGE("Unknown id: %s\n", argv[optind]);
				fprintf(stderr, "Unknown id: %s\n", argv[optind]);
				exit(EXIT_FAILURE);
			}
		} else {
			su_ctx->to.uid = pw->pw_uid;
		}
		optind++;
	}
	if (optind < argc && !strcmp(argv[optind], "--")) {
		optind++;
	}

	// Handle namespaces
	switch (su_ctx->info->mnt_ns) {
	case NAMESPACE_MODE_GLOBAL:
		LOGD("su: use global namespace\n");
		break;
	case NAMESPACE_MODE_REQUESTER:
		LOGD("su: use namespace of pid=[%d]\n", su_ctx->pid);
		if (switch_mnt_ns(su_ctx->pid)) {
			LOGD("su: setns failed, fallback to isolated\n");
			unshare(CLONE_NEWNS);
		}
		break;
	case NAMESPACE_MODE_ISOLATE:
		LOGD("su: use new isolated namespace\n");
		unshare(CLONE_NEWNS);
		break;
	}

	// Change directory to cwd
	chdir(su_ctx->cwd);

	// Check root_access configuration
	switch (su_ctx->info->root_access) {
	case ROOT_ACCESS_DISABLED:
		LOGE("Root access is disabled!\n");
		exit(EXIT_FAILURE);
	case ROOT_ACCESS_APPS_ONLY:
		if (su_ctx->info->uid == UID_SHELL) {
			LOGE("Root access is disabled for ADB!\n");
			exit(EXIT_FAILURE);
		}
		break;
	case ROOT_ACCESS_ADB_ONLY:
		if (su_ctx->info->uid != UID_SHELL) {
			LOGE("Root access limited to ADB only!\n");
			exit(EXIT_FAILURE);
		}
		break;
	case ROOT_ACCESS_APPS_AND_ADB:
	default:
		break;
	}

	// New request or no db exist, notify user for response
	if (su_ctx->info->policy == QUERY) {
		socket_serv_fd = socket_create_temp(su_ctx->sock_path, sizeof(su_ctx->sock_path));
		setup_sighandlers(cleanup_signal);

		// Start activity
		app_send_request(su_ctx);

		atexit(socket_cleanup);

		fd = socket_accept(socket_serv_fd);
		socket_send_request(fd, su_ctx);
		socket_receive_result(fd, result, sizeof(result));

		close(fd);
		close(socket_serv_fd);
		socket_cleanup();

		if (strcmp(result, "socket:ALLOW") == 0)
			su_ctx->info->policy = ALLOW;
		else
			su_ctx->info->policy = DENY;

		// Report the policy to main daemon
		xwrite(su_ctx->pipefd[1], &su_ctx->info->policy, sizeof(su_ctx->info->policy));
		close(su_ctx->pipefd[0]);
		close(su_ctx->pipefd[1]);
	}

	if (su_ctx->info->policy == ALLOW)
		allow();
	else
		deny();
}

