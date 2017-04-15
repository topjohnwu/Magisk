/* misc.c - Miscellaneous stuffs for su
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "magisk.h"
#include "su.h"

int quit_signals[] = { SIGALRM, SIGABRT, SIGHUP, SIGPIPE, SIGQUIT, SIGTERM, SIGINT, 0 };

void setup_sighandlers(void (*handler)(int)) {
	struct sigaction act;

	// Install the termination handlers
	// Note: we're assuming that none of these signal handlers are already trapped.
	// If they are, we'll need to modify this code to save the previous handler and
	// call it after we restore stdin to its previous state.
	memset(&act, 0, sizeof(act));
	act.sa_handler = handler;
	for (int i = 0; quit_signals[i]; ++i) {
		sigaction(quit_signals[i], &act, NULL);
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

char *get_command(const struct su_request *to) {
	if (to->command)
		return to->command;
	if (to->shell)
		return to->shell;
	return DEFAULT_SHELL;
}

int fork_zero_fucks() {
	int pid = fork();
	if (pid) {
		int status;
		waitpid(pid, &status, 0);
		return pid;
	} else {
		if (fork() != 0)
			exit(0);
		return 0;
	}
}
