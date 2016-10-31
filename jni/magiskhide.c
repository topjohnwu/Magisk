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

FILE *logfile;
int i, list_size;
char **hide_list = NULL;
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

void lazy_unmount(const char* mountpoint) {
	if (umount2(mountpoint, MNT_DETACH) != -1)
		fprintf(logfile, "MagiskHide: Unmounted (%s)\n", mountpoint);
	else
		fprintf(logfile, "MagiskHide: Unmount Failed (%s)\n", mountpoint);
}

//WARNING: Calling this will change our current namespace
//We don't care because we don't want to run from here anyway
int hideMagisk(int pid) {
	char *path = NULL;
	asprintf(&path, "/proc/%d/ns/mnt", pid);
	int fd = open(path, O_RDONLY);
	if(fd == -1) return 2;
	if(setns(fd, 0) == -1) return 3;

	free(path);
	path = NULL;
	asprintf(&path, "/proc/%d/mounts", pid);
	FILE *mount_fp = fopen(path, "r");
	if (mount_fp == NULL) {
		fprintf(logfile, "Error opening mount list!\n");
		return 1;
	}
	free(path);

	int mount_size;
	char **mount_list = file_to_str_arr(mount_fp, &mount_size), mountpoint[256], *sbstr;
	fclose(mount_fp);

	for(i = mount_size - 1; i >= 0; --i) {
		if(strstr(mount_list[i], "/dev/block/loop")) {
			sscanf(mount_list[i], "%256s %256s", mountpoint, mountpoint);
			if (!strstr(mountpoint, "/dev/magisk/dummy")) 
				lazy_unmount(mountpoint);
		} else if ((sbstr = strstr(mount_list[i], "/dev/magisk/dummy"))) {
			sscanf(sbstr, "/dev/magisk/dummy%256s", mountpoint);
			lazy_unmount(mountpoint);
		}
		free(mount_list[i]);
	}
	// Free memory
	free(mount_list);

	return 0;
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
	fprintf(logfile, "MagiskHide: Update process/package list:\n");
	for(i = 0; i < list_size; i++)
		fprintf(logfile, "MagiskHide: %s\n", hide_list[i]);
}

void quit_pthread(int sig) {
	// Free memory
	for(i = 0; i < list_size; ++i)
		free(hide_list[i]);
	free(hide_list);
	pthread_exit(NULL);
}

void *monitor_list(void *listpath) {
	signal(SIGQUIT, quit_pthread);
	// Initial load
	update_list((char*) listpath);

	int inotifyFd;
	char buffer[512];

	inotifyFd = inotify_init();
	if (inotifyFd == -1)
		exit(1);
	if (inotify_add_watch(inotifyFd, (char*) listpath, IN_ATTRIB | IN_MODIFY) == -1)
		exit(1);

	while(1) {
		if (read(inotifyFd, buffer, 512) == -1)
			exit(1);
		update_list((char*) listpath);
	}

	return NULL;
}

int main(int argc, char **argv, char **envp) {

	pid_t forkpid = fork();

	if (forkpid < 0)
		return 1;

	if (forkpid == 0) {
		if (setsid() < 0)
			return 1;

		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		logfile = fopen("/cache/magisk.log", "a+");
		setbuf(logfile, NULL);

		pthread_t list_monitor, unmount_thread;

		pthread_mutex_init(&mutex, NULL);
		pthread_create(&list_monitor, NULL, monitor_list, "/magisk/.core/magiskhide/hidelist");

		char buffer[512];
		FILE *p = popen("while true; do logcat -b events -v raw -s am_proc_start; sleep 1; done", "r");

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
					fprintf(logfile, "MagiskHide: Disabling for process = %s, PID = %d, UID = %d\n", processName, pid, uid);
					forkpid = fork();
					if (forkpid < 0)
						break;
					if (forkpid == 0) {
						hideMagisk(pid);
						return 0;
					}
					wait(&i);
					kill(forkpid, SIGTERM);
					break;
				}
			}
		}

		pclose(p);

		pthread_kill(list_monitor, SIGQUIT);
		pthread_mutex_destroy(&mutex);

		fprintf(logfile, "MagiskHide: Error occurred...\n");

		fclose(logfile);

		return 1;
	}

	return 0;
}
