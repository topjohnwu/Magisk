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
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <selinux/selinux.h>

#include "magisk.h"
#include "utils.h"
#include "su.h"

struct su_context *su_ctx;

static void usage(int status) {
	FILE *stream = (status == EXIT_SUCCESS) ? stdout : stderr;

	fprintf(stream,
	"MagiskSU v" xstr(MAGISK_VERSION) "(" xstr(MAGISK_VER_CODE) ")\n\n"
	"Usage: su [options] [-] [user [argument...]]\n\n"
	"Options:\n"
	"  -c, --command COMMAND         pass COMMAND to the invoked shell\n"
	"  -h, --help                    display this help message and exit\n"
	"  -, -l, --login                pretend the shell to be a login shell\n"
	"  -m, -p,\n"
	"  --preserve-environment        preserve the entire environment\n"
	"  -s, --shell SHELL             use SHELL instead of the default " DEFAULT_SHELL "\n"
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
		if (command[0])
			sprintf(command, "%s %s", command, argv[i]);
		else
			strcpy(command, argv[i]);
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
	char* argv[] = { NULL, NULL, NULL, NULL };

	if (su_ctx->info->access.notify || su_ctx->info->access.log)
		app_send_result(su_ctx, ALLOW);

	if (su_ctx->to.login)
		argv[0] = "-";
	else
		argv[0] = basename(su_ctx->to.shell);

	if (su_ctx->to.command) {
		argv[1] = "-c";
		argv[2] = su_ctx->to.command;
	}

	// Setup shell
	umask(022);
	populate_environment(su_ctx);
	set_identity(su_ctx->to.uid);

	execvp(su_ctx->to.shell, argv);
	fprintf(stderr, "Cannot execute %s: %s\n", su_ctx->to.shell, strerror(errno));
	PLOGE("exec");
	exit(EXIT_FAILURE);
}

static __attribute__ ((noreturn)) void deny() {
	if (su_ctx->info->access.notify || su_ctx->info->access.log)
		app_send_result(su_ctx, DENY);

	LOGW("su: request rejected (%u->%u)", su_ctx->info->uid, su_ctx->to.uid);
	fprintf(stderr, "%s\n", strerror(EACCES));
	exit(EXIT_FAILURE);
}

static void socket_cleanup() {
	if (su_ctx && su_ctx->sock_path[0]) {
		unlink(su_ctx->sock_path);
		su_ctx->sock_path[0] = '\0';
	}
}

static void cleanup_signal(int sig) {
	socket_cleanup();
	exit2(EXIT_FAILURE);
}

__attribute__ ((noreturn)) void exit2(int status) {
	// Handle the pipe, or the daemon will get stuck
	if (su_ctx->pipefd[0] >= 0) {
		xwrite(su_ctx->pipefd[1], &su_ctx->info->access.policy, sizeof(policy_t));
		close(su_ctx->pipefd[0]);
		close(su_ctx->pipefd[1]);
	}
	exit(status);
}

int su_daemon_main(int argc, char **argv) {
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
		case 'z':
			// Do nothing, placed here for legacy support :)
			break;
		case 'M':
			su_ctx->info->dbs.v[SU_MNT_NS] = NAMESPACE_MODE_GLOBAL;
			break;
		default:
			/* Bionic getopt_long doesn't terminate its error output by newline */
			fprintf(stderr, "\n");
			usage(2);
		}
	}

	if (optind < argc && strcmp(argv[optind], "-") == 0) {
		su_ctx->to.login = 1;
		optind++;
	}
	/* username or uid */
	if (optind < argc) {
		struct passwd *pw;
		pw = getpwnam(argv[optind]);
		if (pw)
			su_ctx->to.uid = pw->pw_uid;
		else
			su_ctx->to.uid = atoi(argv[optind]);
		optind++;
	}

	// Handle namespaces
	switch (su_ctx->info->dbs.v[SU_MNT_NS]) {
	case NAMESPACE_MODE_GLOBAL:
		LOGD("su: use global namespace\n");
		break;
	case NAMESPACE_MODE_REQUESTER:
		LOGD("su: use namespace of pid=[%d]\n", su_ctx->pid);
		if (switch_mnt_ns(su_ctx->pid)) {
			LOGD("su: setns failed, fallback to isolated\n");
			xunshare(CLONE_NEWNS);
		}
		break;
	case NAMESPACE_MODE_ISOLATE:
		LOGD("su: use new isolated namespace\n");
		xunshare(CLONE_NEWNS);
		break;
	}

	// Change directory to cwd
	chdir(su_ctx->cwd);

	// New request or no db exist, notify user for response
	if (su_ctx->pipefd[0] >= 0) {
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
			su_ctx->info->access.policy = ALLOW;
		else
			su_ctx->info->access.policy = DENY;

		// Report the policy to main daemon
		xwrite(su_ctx->pipefd[1], &su_ctx->info->access.policy, sizeof(policy_t));
		close(su_ctx->pipefd[0]);
		close(su_ctx->pipefd[1]);
	}

	if (su_ctx->info->access.policy == ALLOW)
		allow();
	else
		deny();
}

