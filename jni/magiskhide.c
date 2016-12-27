#define _GNU_SOURCE
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include <sys/mount.h>
#include <sys/inotify.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#define LOGFILE 	"/cache/magisk.log"
#define HIDELIST 	"/magisk/.core/magiskhide/hidelist"

FILE *logfile;
int i, list_size, pipefd[2], zygote_num = 0;
char **hide_list = NULL;
char zygote_ns[2][32];
pthread_mutex_t mutex;

char **file_to_str_arr(FILE *fp, int *size) {
	int allocated = 16;
	char *line = NULL, **array;
	size_t len = 0;
	ssize_t read;

	array = (char **) malloc(sizeof(char*) * allocated);

	*size = 0;
	while ((read = getline(&line, &len, fp)) != -1) {
		if (*size >= allocated) {
			// Double our allocation and re-allocate
			allocated *= 2;
			array = (char **) realloc(array, sizeof(char*) * allocated);
		}
		// Remove end newline
		if (line[read - 1] == '\n') {
			line[read - 1] = '\0';
		}
		array[*size] = line;
		line = NULL;
		++(*size);
	}
	return array;
}

void read_namespace(const int pid, char* target, const size_t size) {
	char path[32];
	snprintf(path, sizeof(path), "/proc/%d/ns/mnt", pid);
	ssize_t len = readlink(path, target, size);
	target[len] = '\0';
}

void lazy_unmount(const char* mountpoint) {
	if (umount2(mountpoint, MNT_DETACH) != -1)
		fprintf(logfile, "MagiskHide: Unmounted (%s)\n", mountpoint);
	else
		fprintf(logfile, "MagiskHide: Unmount Failed (%s)\n", mountpoint);
}

int hideMagisk() {
	int pid;
	char path[256], cache_block[256], namespace[32];
	cache_block[0] = 0;

	close(pipefd[1]);
	while(1) {
		read(pipefd[0], &pid, sizeof(pid));
		if(pid == -1) break;

		int badns;
		do {
			badns = 0;
			read_namespace(pid, namespace, 32);
			for (i = 0; i < zygote_num; ++i) {
				if (strcmp(namespace, zygote_ns[i]) == 0) {
					usleep(50000);
					badns = 1;
					break;
				}
			}
			if (badns) continue;
			break;
		} while(1);

		fprintf(logfile, "ns=%s\n", namespace);

		snprintf(path, 256, "/proc/%d/ns/mnt", pid);

		int fd = open(path, O_RDONLY);
		if(fd == -1) continue; // Maybe process died..
		if(setns(fd, 0) == -1) {
			fprintf(logfile, "MagiskHide: Unable to change namespace for pid=%d\n", pid);
			continue;
		}
		close(fd);

		snprintf(path, 256, "/proc/%d/mounts", pid);
		FILE *mount_fp = fopen(path, "r");
		if (mount_fp == NULL) {
			fprintf(logfile, "MagiskHide: Error opening mount list!\n");
			continue;
		}

		int mount_size;
		char **mount_list = file_to_str_arr(mount_fp, &mount_size), mountpoint[256], cache_block[256];

		// Find the cache block name if not found yet
		if (strlen(cache_block) == 0) {
			for(i = 0; i < mount_size; ++i) {
				if (strstr(mount_list[i], " /cache ")) {
					sscanf(mount_list[i], "%256s", cache_block);
					break;
				}
			}
		}
		
		// First unmount the dummy skeletons and the cache mounts
		for(i = mount_size - 1; i >= 0; --i) {
			if (strstr(mount_list[i], "tmpfs /system/") || strstr(mount_list[i], "tmpfs /vendor/")
				|| (strstr(mount_list[i], cache_block) && strstr(mount_list[i], "/system")) ) {
				sscanf(mount_list[i], "%*s %256s", mountpoint);
				lazy_unmount(mountpoint);
			}
			free(mount_list[i]);
		}
		// Free memory
		free(mount_list);

		// Re-read mount infos
		fseek(mount_fp, 0, SEEK_SET);
		mount_list = file_to_str_arr(mount_fp, &mount_size);
		fclose(mount_fp);

		// Unmount loop mounts
		for(i = mount_size - 1; i >= 0; --i) {
			if (strstr(mount_list[i], "/dev/block/loop")) {
				sscanf(mount_list[i], "%*s %256s", mountpoint);
				lazy_unmount(mountpoint);
			}
			free(mount_list[i]);
		}
		// Free memory
		free(mount_list);
	}

	// Should never go here
	return 1;
}

void update_list(const char *listpath) {
	FILE *hide_fp = fopen((char*) listpath, "r");
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

void *monitor_list(void *path) {
	char* listpath = (char*) path;
	signal(SIGQUIT, quit_pthread);

	int inotifyFd = -1;
	char buffer[512];

	while(1) {
		if (inotifyFd == -1 || read(inotifyFd, buffer, 512) == -1) {
			close(inotifyFd);
			inotifyFd = inotify_init();
			if (inotifyFd == -1) {
				fprintf(logfile, "MagiskHide: Unable to watch %s\n", listpath);
				exit(1);
			}
			if (inotify_add_watch(inotifyFd, (char*) listpath, IN_MODIFY) == -1) {
				fprintf(logfile, "MagiskHide: Unable to watch %s\n", listpath);
				exit(1);
			}
		}
		update_list(listpath);
	}

	return NULL;
}

void run_as_daemon() {
	switch(fork()) {
		case -1:
			exit(-1);
		case 0:
			if (setsid() < 0)
				exit(-1);
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
			logfile = fopen(LOGFILE, "a+");
			setbuf(logfile, NULL);
			break;
		default:
			exit(0); 
	}
}

int main(int argc, char **argv, char **envp) {

	run_as_daemon();

	// Get the mount namespace of zygote for checking
	char buffer[512];
	int pid;
	FILE *p = popen("/data/busybox/ps | grep zygote | grep -v grep", "r");
	while(fgets(buffer, sizeof(buffer), p)) {
		if (zygote_num == 2) break;
		sscanf(buffer, "%d", &pid);
		read_namespace(pid, zygote_ns[zygote_num], 32);
		++zygote_num;
	}
	pclose(p);

	for (i = 0; i < zygote_num; ++i)
		fprintf(logfile, "Zygote(%d) ns=%s ", i, zygote_ns[i]);
	fprintf(logfile, "\n");

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
	pthread_t list_monitor;
	pthread_mutex_init(&mutex, NULL);
	pthread_create(&list_monitor, NULL, monitor_list, HIDELIST);

	p = popen("while true; do logcat -b events -v raw -s am_proc_start; sleep 1; done", "r");

	while(!feof(p)) {
		//Format of am_proc_start is (as of Android 5.1 and 6.0)
		//UserID, pid, unix uid, processName, hostingType, hostingName
		fgets(buffer, sizeof(buffer), p);

		char *pos = buffer;
		while(1) {
			pos = strchr(pos, ',');
			if(pos == NULL)
				break;
			pos[0] = ' ';
		}

		int user, pid, uid;
		char processName[256], hostingType[16], hostingName[256];
		int ret = sscanf(buffer, "[%d %d %d %256s %16s %256s]",
				&user, &pid, &uid,
				processName, hostingType, hostingName);

		if(ret != 6)
			continue;

		for (i = 0; i < list_size; ++i) {
			if(strstr(processName, hide_list[i])) {
				fprintf(logfile, "MagiskHide: Disabling for process=%s, PID=%d, ", processName, pid, uid);
				write(pipefd[1], &pid, sizeof(pid));
			}
		}
	}

	// Close the logcat monitor
	pclose(p);

	// Close the config list monitor
	pthread_kill(list_monitor, SIGQUIT);
	pthread_mutex_destroy(&mutex);

	// Terminate our children
	i = -1;
	write(pipefd[1], &i, sizeof(i));

	fprintf(logfile, "MagiskHide: Cannot read from logcat, abort...\n");
	fclose(logfile);

	return 1;
}
