/* post_fs_data.c - post-fs-data actions
 */

#include <unistd.h>

#include "utils.h"
#include "daemon.h"

void post_fs_data(int client) {
	// ack
	write_int(client, 0);
	// TODO: Do something
	close(client);
	unblock_boot_process();
}
