#include "magiskhide.h"

void monitor_proc() {
	// Monitor am_proc_start in main thread
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

		int pid;
		char processName[256];
		int ret = sscanf(buffer, "[%*d %d %*d %256s", &pid, processName);

		if(ret != 2)
			continue;

		for (i = 0; i < list_size; ++i) {
			if(strcmp(processName, hide_list[i]) == 0) {
				// Check PID exist
				if (kill(pid, 0) == -1) continue;
				fprintf(logfile, "MagiskHide: %s(PID=%d ", processName, pid);
				write(pipefd[1], &pid, sizeof(pid));
			}
		}
	}

	// Close the logcat monitor
	pclose(p);
}