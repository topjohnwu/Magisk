/*
** Copyright 2013, Kevin Cernekee <cernekee@gmail.com>
**
** This was reverse engineered from an HTC "reboot" binary and is an attempt
** to remain bug-compatible with the original.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sqlite3.h>
#include <sys/reboot.h>
#include <android/log.h>

#define SETTINGS_DB		"/data/data/com.android.providers.settings/databases/settings.db"
#define SCREEN_LOCK_STATUS	"/data/misc/screen_lock_status"

int pattern_lock;

int sqlcallback(void *private, int n_columns, char **col_values, char **col_names)
{
	pattern_lock = 0;

	if (n_columns == 0 || !col_values[0])
		return 0;
	
	if (!strcmp(col_values[0], "1"))
		pattern_lock = 1;

	__android_log_print(ANDROID_LOG_INFO, NULL,
			    "sqlcallback %s = %s, pattern_locks= %d\n",
			    col_names[0], col_values[0], pattern_lock);
	return 0;
}

int checkPatternLock(void)
{
	sqlite3 *pDb = NULL;
	char *errmsg = NULL;

	if (sqlite3_open(SETTINGS_DB, &pDb) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, NULL,
				    "sqlite3_open error");
		/* BUG? probably shouldn't call sqlite3_close() if open failed */
		goto out;
	}

	if (sqlite3_exec(pDb,
			 "select value from system where name= \"lock_pattern_autolock\"",
			 sqlcallback, "checkPatternLock", &errmsg) != 0) {
		__android_log_print(ANDROID_LOG_ERROR, NULL,
				    "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		goto out;
	}

out:
	sqlite3_close(pDb);
	return 0;
}

int main(int argc, char **argv)
{
	int no_sync = 0, power_off = 0;
	int ret;

	opterr = 0;
	while ((ret = getopt(argc, argv, "np")) != -1) {
		switch (ret) {
		case 'n':
			no_sync = 1;
			break;
		case 'p':
			power_off = 1;
			break;
		case '?':
			fprintf(stderr, "usage: %s [-n] [-p] [rebootcommand]\n",
				argv[0]);
			exit(1);
			break;
		}
	}

	if (argc > (optind + 1)) {
		fprintf(stderr, "%s: too many arguments\n", argv[0]);
		exit(1);
	}

	/* BUG: this should use optind */
	if (argc > 1 && !strcmp(argv[1], "oem-78")) {
		/* HTC RUU mode: "reboot oem-78" */

		FILE *f;
		char buf[5];

		checkPatternLock();
		f = fopen(SCREEN_LOCK_STATUS, "r");
		if (!f) {
			fputs("5\n", stderr);
			exit(0);
		}
		fgets(buf, 5, f);
		if (atoi(buf) == 1) {
			if (pattern_lock != 0) {
				fputs("1\n", stderr);
				exit(0);
			}
		}
		fputs("0\n", stderr);
	}

	if (!no_sync)
		sync();

	if (power_off) {
		ret = __reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2,
			       LINUX_REBOOT_CMD_POWER_OFF, 0);
	} else if (optind >= argc) {
		ret = reboot(LINUX_REBOOT_CMD_RESTART);
	} else {
		ret = __reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2,
			       LINUX_REBOOT_CMD_RESTART2, argv[optind]);
	}

	if (ret < 0) {
		perror("reboot");
		exit(1);
	} else {
		fputs("reboot returned\n", stderr);
		exit(0);
	}
}
