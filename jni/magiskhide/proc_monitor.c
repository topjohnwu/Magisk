#include "magiskhide.h"

void monitor_proc() {
	int pid, badns, i, zygote_num = 0;
	char init_ns[32], zygote_ns[2][32];

	// Get the mount namespace of init
	read_namespace(1, init_ns, 32);

	printf("%s\n", init_ns);

	// Get the mount namespace of zygote
	FILE *p = popen("ps | grep zygote | grep -v grep", "r");
	while(fgets(buffer, sizeof(buffer), p)) {
		if (zygote_num == 2) break;
		sscanf(buffer, "%d", &pid);
		do {
			usleep(500);
			read_namespace(pid, zygote_ns[zygote_num], 32);
		} while (strcmp(zygote_ns[zygote_num], init_ns) == 0);
		++zygote_num;
	}
	pclose(p);

	for (i = 0; i < zygote_num; ++i)
		fprintf(logfile, "Zygote(%d) ns=%s ", i, zygote_ns[i]);
	fprintf(logfile, "\n");

	// get a sample line from am_proc_start
	p = popen("logcat -b events -v raw -s am_proc_start -t 1", "r");

	/**
	 *	Format of am_proc_start is (as of Android 5.1 and 6.0)
	 *	UserID, pid, unix uid, processName, hostingType, hostingName
	 *	but sometimes can have 7 fields, with processName as 5th field
	 */
	fgets(buffer, sizeof(buffer), p);
	int commas = 0;
	char *s = buffer;
	for (i = 0;s[i] != '\0';i++) {
		if (s[i] == ',')
			commas++;

	}
	int numFields = commas + 1;

	pclose(p);

	// Monitor am_proc_start
	p = popen("logcat -b events -c; logcat -b events -v raw -s am_proc_start", "r");

	while(!feof(p)) {
		fgets(buffer, sizeof(buffer), p);

		char *pos = buffer;
		while(1) {
			pos = strchr(pos, ',');
			if(pos == NULL)
				break;
			pos[0] = ' ';
		}

		char processName[256];
		int ret;

		if (numFields == 7) {
			ret = sscanf(buffer, "[%*d %d %*d %*d %256s", &pid, processName);
		} else {
			ret = sscanf(buffer, "[%*d %d %*d %256s", &pid, processName);
		}

		if(ret != 2)
			continue;

		pthread_mutex_lock(&mutex);
		for (i = 0; i < list_size; ++i) {
			if(strcmp(processName, hide_list[i]) == 0) {
				while(1) {
					badns = 0;
					read_namespace(pid, buffer, 32);
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

				fprintf(logfile, "MagiskHide: %s(PID=%d ns=%s)\n", processName, pid, buffer);

				// Unmount start
				write(pipefd[1], &pid, sizeof(pid));
				break;
			}
		}
		pthread_mutex_unlock(&mutex);
	}

	// Close the logcat monitor
	pclose(p);
}
