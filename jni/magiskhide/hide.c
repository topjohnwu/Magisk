#include "magiskhide.h"

int hideMagisk() {
	close(pipefd[1]);

	int pid, zygote_num = 0;
	char cache_block[256], zygote_ns[2][32];
	cache_block[0] = '\0';

	// Set to the top priority
	setpriority(PRIO_PROCESS, 0, -20);

	// Get the mount namespace of zygote
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

	while(1) {
		read(pipefd[0], &pid, sizeof(pid));
		// Termination called
		if(pid == -1) break;

		int badns, fd;
		while(1) {
			badns = 0;
			read_namespace(pid, buffer, 32);
			printf("%s\n", buffer);
			for (i = 0; i < zygote_num; ++i) {
				if (strcmp(buffer, zygote_ns[i]) == 0) {
					usleep(500);
					badns = 1;
					break;
				}
			}
			if (!badns) break;
		}

		// Send pause signal ASAP
		if (kill(pid, SIGSTOP) == -1) continue;

		fprintf(logfile, "ns=%s)\n", buffer);

		snprintf(buffer, sizeof(buffer), "/proc/%d/ns/mnt", pid);
		if((fd = open(buffer, O_RDONLY)) == -1) continue; // Maybe process died..
		if(setns(fd, 0) == -1) {
			fprintf(logfile, "MagiskHide: Unable to change namespace for pid=%d\n", pid);
			continue;
		}
		close(fd);

		snprintf(buffer, sizeof(buffer), "/proc/%d/mounts", pid);
		FILE *mount_fp = fopen(buffer, "r");
		if (mount_fp == NULL) {
			fprintf(logfile, "MagiskHide: Error opening mount list!\n");
			continue;
		}

		int mount_size;
		char **mount_list = file_to_str_arr(mount_fp, &mount_size);

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
				sscanf(mount_list[i], "%*s %512s", buffer);
				lazy_unmount(buffer);
			}
			free(mount_list[i]);
		}
		free(mount_list);

		// Re-read mount infos
		fseek(mount_fp, 0, SEEK_SET);
		mount_list = file_to_str_arr(mount_fp, &mount_size);
		fclose(mount_fp);

		// Unmount loop mounts
		for(i = mount_size - 1; i >= 0; --i) {
			if (strstr(mount_list[i], "/dev/block/loop") && !strstr(mount_list[i], DUMMYPATH)) {
				sscanf(mount_list[i], "%*s %512s", buffer);
				lazy_unmount(buffer);
			}
			free(mount_list[i]);
		}
		free(mount_list);

		// Send resume signal
		kill(pid, SIGCONT);
	}

	// Should never go here
	return 1;
}
