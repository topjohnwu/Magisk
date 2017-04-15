/* post_fs.c - post-fs actions
 */

#include <unistd.h>

#include "utils.h"
#include "daemon.h"

void post_fs(int client) {
	// ack
	write_int(client, 0);
	// TODO: Do something
	close(client);
	unblock_boot_process();
}
