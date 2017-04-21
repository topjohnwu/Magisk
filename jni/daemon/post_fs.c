/* post_fs.c - post-fs actions
 */

#include <unistd.h>

#include "magisk.h"
#include "utils.h"
#include "daemon.h"

void post_fs(int client) {
	LOGI("** post-fs mode running\n");
	// ack
	write_int(client, 0);

	// TODO: Simple bind mounts

	close(client);

unblock:
	unblock_boot_process();
}
