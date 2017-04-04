#include "magiskhide.h"

void *monitor_list(void *path) {
	char* listpath = (char*) path;
	signal(SIGQUIT, quit_pthread);

	int inotifyFd = -1;
	char str[512];

	while(1) {
		if (inotifyFd == -1 || read(inotifyFd, str, sizeof(str)) == -1) {
			close(inotifyFd);
			inotifyFd = inotify_init();
			if (inotifyFd == -1) {
				fprintf(logfile, "MagiskHide: Unable to watch %s\n", listpath);
				exit(1);
			}
			if (inotify_add_watch(inotifyFd, listpath, IN_CLOSE_WRITE) == -1) {
				fprintf(logfile, "MagiskHide: Unable to watch %s\n", listpath);
				exit(1);
			}
		}
		update_list(listpath);
	}

	return NULL;
}

void update_list(const char *listpath) {
	FILE *hide_fp = fopen(listpath, "r");
	if (hide_fp == NULL) {
		fprintf(logfile, "MagiskHide: Error opening hide list\n");
		exit(1);
	}
	pthread_mutex_lock(&mutex);
	if (hide_list) {
		// Free memory
		for(i = 0; i < list_size; ++i)
			free(hide_list[i]);
		free(hide_list);
	}
	hide_list = file_to_str_arr(hide_fp, &list_size);
	pthread_mutex_unlock(&mutex);
	fclose(hide_fp);
	if (list_size) fprintf(logfile, "MagiskHide: Update process/package list:\n");
	for(i = 0; i < list_size; i++)
		fprintf(logfile, "MagiskHide: [%s]\n", hide_list[i]);
}

void quit_pthread(int sig) {
	// Free memory
	for(i = 0; i < list_size; ++i)
		free(hide_list[i]);
	free(hide_list);
	pthread_exit(NULL);
}
