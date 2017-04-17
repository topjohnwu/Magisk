/* late_start.c - late_start service actions
 */

#include <unistd.h>
#include <pthread.h>

#include "magisk.h"
#include "daemon.h"

void late_start(int client) {
	LOGI("** late_start service mode running\n");
	// ack
	write_int(client, 0);
	// TODO: Do something
	close(client);

	// Wait till the full patch is done
	pthread_join(sepol_patch, NULL);
}
