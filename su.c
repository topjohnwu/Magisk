/*
 * Copyright 2017, John Wu (@topjohnwu)
 * Copyright 2015, Pierre-Hugues Husson <phh@phh.me>
 * Copyright 2010, Adam Shanks (@ChainsDD)
 * Copyright 2008, Zinx Verituse (@zinxv)
 */

/* su.c - The main function running in the daemon
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <pwd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/auxv.h>
#include <selinux/selinux.h>

#include "magisk.h"
#include "utils.h"
#include "resetprop.h"
#include "su.h"

static struct su_context *su_ctx;

static void usage(int status) {
	FILE *stream = (status == EXIT_SUCCESS) ? stdout : stderr;

	fprintf(stream,
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
	"                                this is used almost exclusively by Superuser.apk\n");
	exit(status);
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

static int get_multiuser_mode() {
	// TODO: Multiuser support
	return MULTIUSER_MODE_NONE;
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

static __attribute__ ((noreturn)) void allow() {
	char *arg0;

	umask(su_ctx->umask);

	// no need to log if called by root
	if (su_ctx->from.uid != UID_ROOT)
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
	// no need to log if called by root
	if (su_ctx->from.uid != UID_ROOT)
		app_send_result(su_ctx, DENY);

	LOGW("su: request rejected (%u->%u)", su_ctx->from.uid, su_ctx->to.uid);
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
	exit(EXIT_FAILURE);
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

	// Replace -cn with z, for CF compatibility
	for (int i = 0; i < argc; ++i) {
		if (strcmp(argv[i], "-cn") == 0) {
			strcpy(argv[i], "-z");
			break;
		}
	}

	// Default values
	struct su_context ctx = {
		.from = {
			.pid = from_pid,
			.uid = from_uid,
		},
		.to = {
			.uid = UID_ROOT,
			.login = 0,
			.keepenv = 0,
			.shell = DEFAULT_SHELL,
			.command = NULL,
			.argv = argv,
			.argc = argc,
		},
		.user = {
			.android_user_id = 0,
			.multiuser_mode = get_multiuser_mode(),
			.database_path = APPLICATION_DATA_PATH REQUESTOR_DATABASE_PATH,
			.base_path = APPLICATION_DATA_PATH REQUESTOR
		},
		.umask = umask(027),
	};
	su_ctx = &ctx;

	struct stat st;
	int c, socket_serv_fd, fd;
	char result[64];
	policy_t dballow;
	struct option long_opts[] = {
		{ "command",                required_argument,  NULL, 'c' },
		{ "help",                   no_argument,        NULL, 'h' },
		{ "login",                  no_argument,        NULL, 'l' },
		{ "preserve-environment",   no_argument,        NULL, 'p' },
		{ "shell",                  required_argument,  NULL, 's' },
		{ "version",                no_argument,        NULL, 'v' },
		{ "context",                required_argument,  NULL, 'z' },
		{ NULL, 0, NULL, 0 },
	};

	while ((c = getopt_long(argc, argv, "c:hlmps:Vvuz:", long_opts, NULL)) != -1) {
		switch (c) {
		case 'c':
			ctx.to.command = concat_commands(argc, argv);
			optind = argc;
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		case 'l':
			ctx.to.login = 1;
			break;
		case 'm':
		case 'p':
			ctx.to.keepenv = 1;
			break;
		case 's':
			ctx.to.shell = optarg;
			break;
		case 'V':
			printf("%d\n", VERSION_CODE);
			exit(EXIT_SUCCESS);
		case 'v':
			printf("%s\n", SU_VERSION_STR);
			exit(EXIT_SUCCESS);
		case 'u':
			switch (ctx.user.multiuser_mode) {
			case MULTIUSER_MODE_USER:
				printf("%s\n", MULTIUSER_VALUE_USER);
				break;
			case MULTIUSER_MODE_OWNER_MANAGED:
				printf("%s\n", MULTIUSER_VALUE_OWNER_MANAGED);
				break;
			case MULTIUSER_MODE_OWNER_ONLY:
				printf("%s\n", MULTIUSER_VALUE_OWNER_ONLY);
				break;
			case MULTIUSER_MODE_NONE:
				printf("%s\n", MULTIUSER_VALUE_NONE);
				break;
			}
			exit(EXIT_SUCCESS);
		case 'z':
			// Do nothing, placed here for legacy support :)
			break;
		default:
			/* Bionic getopt_long doesn't terminate its error output by newline */
			fprintf(stderr, "\n");
			usage(2);
		}
	}
	if (optind < argc && !strcmp(argv[optind], "-")) {
		ctx.to.login = 1;
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
			ctx.to.uid = strtoul(argv[optind], &endptr, 10);
			if (errno || *endptr) {
				LOGE("Unknown id: %s\n", argv[optind]);
				fprintf(stderr, "Unknown id: %s\n", argv[optind]);
				exit(EXIT_FAILURE);
			}
		} else {
			ctx.to.uid = pw->pw_uid;
		}
		optind++;
	}
	if (optind < argc && !strcmp(argv[optind], "--")) {
		optind++;
	}

	// The su_context setup is done, now every error leads to deny
	err_handler = deny;

	// It's in multiuser mode
	if (ctx.from.uid > 99999) {
		ctx.user.android_user_id = ctx.from.uid / 100000;
		if (ctx.user.multiuser_mode == MULTIUSER_MODE_USER) {
			snprintf(ctx.user.database_path, PATH_MAX, "%s/%d/%s",
				USER_DATA_PATH, ctx.user.android_user_id, REQUESTOR_DATABASE_PATH);
			snprintf(ctx.user.base_path, PATH_MAX, "%s/%d/%s",
				USER_DATA_PATH, ctx.user.android_user_id, REQUESTOR);
		}
	}

	// verify superuser is installed
	xstat(ctx.user.base_path, &st);

	// odd perms on superuser data dir
	if (st.st_gid != st.st_uid) {
		LOGE("Bad uid/gid %d/%d for Superuser Requestor application",
				(int)st.st_uid, (int)st.st_gid);
		deny();
	}

	// always allow if this is the superuser uid
	// superuser needs to be able to reenable itself when disabled...
	if (ctx.from.uid == st.st_uid) {
		allow();
	}

	// Check property of root configuration
	char *root_prop = getprop(ROOT_ACCESS_PROP);
	if (root_prop) {
		int prop_status = atoi(root_prop);
		switch (prop_status) {
		case ROOT_ACCESS_DISABLED:
			exit(EXIT_FAILURE);
		case ROOT_ACCESS_APPS_ONLY:
			if (ctx.from.uid == UID_SHELL)
				exit(EXIT_FAILURE);
			break;
		case ROOT_ACCESS_ADB_ONLY:
			if (ctx.from.uid != UID_SHELL)
				exit(EXIT_FAILURE);
			break;
		case ROOT_ACCESS_APPS_AND_ADB:
		default:
			break;
		}
	} else {
		exit(EXIT_FAILURE);
	}
	free(root_prop);

	// Allow root to start root
	if (ctx.from.uid == UID_ROOT) {
		allow();
	}

	// deny if this is a non owner request and owner mode only
	if (ctx.user.multiuser_mode == MULTIUSER_MODE_OWNER_ONLY && ctx.user.android_user_id != 0) {
		deny();
	}

	mkdir(REQUESTOR_CACHE_PATH, 0770);
	if (chown(REQUESTOR_CACHE_PATH, st.st_uid, st.st_gid)) {
		PLOGE("chown (%s, %u, %u)", REQUESTOR_CACHE_PATH, st.st_uid, st.st_gid);
	}

	if (setgroups(0, NULL)) {
		PLOGE("setgroups");
	}
	if (setegid(st.st_gid)) {
		PLOGE("setegid (%u)", st.st_gid);
	}
	if (seteuid(st.st_uid)) {
		PLOGE("seteuid (%u)", st.st_uid);
	}

	// If db exits, check directly instead query application
	dballow = database_check(&ctx);
	switch (dballow) {
	case INTERACTIVE:
		break;
	case ALLOW:
		allow();
	case DENY:
	default:
		deny();
	}

	// New request or no db exist, notify app for response
	socket_serv_fd = socket_create_temp(ctx.sock_path, sizeof(ctx.sock_path));
	setup_sighandlers(cleanup_signal);

	// Start activity
	app_send_request(su_ctx);

	atexit(socket_cleanup);

	fd = socket_accept(socket_serv_fd);
	socket_send_request(fd, &ctx);
	socket_receive_result(fd, result, sizeof(result));

	close(fd);
	close(socket_serv_fd);
	socket_cleanup();

	if (strcmp(result, "socket:DENY") == 0) {
		deny();
	} else if (strcmp(result, "socket:ALLOW") == 0) {
		allow();
	} else {
		LOGE("unknown response from Superuser Requestor: %s", result);
		deny();
	}
}

