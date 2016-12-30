#include "magiskhide.h"

FILE *logfile;
int i, list_size, pipefd[2];
char **hide_list = NULL, buffer[512];
pthread_t list_monitor;
pthread_mutex_t mutex;

static void terminate(int sig) {
	// Close the config list monitor
	pthread_kill(list_monitor, SIGQUIT);
	pthread_mutex_destroy(&mutex);

	// Terminate our children
	i = -1;
	write(pipefd[1], &i, sizeof(i));
}

int main(int argc, char **argv, char **envp) {

	run_as_daemon();

	// Handle all killing signals
	signal(SIGINT, terminate);
	signal(SIGKILL, terminate);
	signal(SIGTERM, terminate);

	// Fork a child to handle namespace switches and unmounts
	pipe(pipefd);
	switch(fork()) {
		case -1:
			exit(-1);
		case 0:
			return hideMagisk();
		default:
			break; 
	}
	close(pipefd[0]);

	// Start a thread to constantly check the hide list
	pthread_mutex_init(&mutex, NULL);
	pthread_create(&list_monitor, NULL, monitor_list, HIDELIST);

	monitor_proc();

	terminate(0);

	fprintf(logfile, "MagiskHide: Cannot monitor am_proc_start, abort...\n");
	fclose(logfile);

	return 1;
}