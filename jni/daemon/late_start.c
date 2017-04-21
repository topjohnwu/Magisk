/* late_start.c - late_start service actions
 */

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "magisk.h"
#include "daemon.h"
#include "utils.h"

void late_start(int client) {
	LOGI("** late_start service mode running\n");
	// ack
	write_int(client, 0);
	close(client);

	// Wait till the full patch is done
	pthread_join(sepol_patch, NULL);

	// Run scripts after full patch, most reliable way to run scripts
	exec_common_script("service");
}
