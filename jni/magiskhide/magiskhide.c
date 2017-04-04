#include "magiskhide.h"

#ifdef INDEP_BINARY
int magiskhide_main(int argc, char *argv[]);
int main(int argc, char *argv[]) {
	return magiskhide_main(argc, argv);
}
#else
#include "magisk.h"
#endif

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
	exit(0);
}

int magiskhide_main(int argc, char *argv[]) {

	if (argc > 1) {
		if (strcmp(argv[1], "--daemon") == 0)
			run_as_daemon();
		else {
			fprintf(stderr, "%s (with no options)\n\tRun magiskhide and output to stdout\n", argv[0]);
			fprintf(stderr, "%s --daemon\n\tRun magiskhide as daemon, output to magisk.log\n", argv[0]);
			return 1;
		}
	} else 
		logfile = stdout;


	// Handle all killing signals
	signal(SIGINT, terminate);
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

	// Set main process to the top priority
	setpriority(PRIO_PROCESS, 0, -20);

	monitor_proc();

	terminate(0);

	fprintf(logfile, "MagiskHide: Cannot monitor am_proc_start, abort...\n");
	fclose(logfile);

	return 1;
}