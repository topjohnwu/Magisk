/* magiskhide.c - initialize the environment for Magiskhide
 */

// TODO: Functions to modify hiding list

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "magisk.h"
#include "utils.h"
#include "magiskhide.h"

int pipefd[2];
struct vector *hide_list, *new_list;

static pthread_t proc_monitor_thread;

void launch_magiskhide() {
	/*
	 * The setns system call do not support multithread processes
	 * We have to fork a new process, and communicate with pipe
	 */

	xpipe(pipefd);

	// Launch the hide daemon
	hide_daemon();

	close(pipefd[0]);

	// Initialize the hide list
	hide_list = new_list = malloc(sizeof(*hide_list));
	vec_init(hide_list);
	FILE *fp = xfopen(HIDELIST, "r");
	file_to_vector(hide_list, fp);
	fclose(fp);

	// Start a new thread to monitor processes
	pthread_create(&proc_monitor_thread, NULL, proc_monitor, NULL);
}

void stop_magiskhide() {
	int kill = -1;
	// Terminate hide daemon
	xwrite(pipefd[1], &kill, sizeof(kill));
	// Stop process monitor
	pthread_kill(proc_monitor_thread, SIGUSR1);
	pthread_join(proc_monitor_thread, NULL);
}

int magiskhide_main(int argc, char *argv[]) {
	launch_magiskhide();
	return 0;
}
