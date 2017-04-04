/* main.c - The entry point, should be mutli-call
 */

#include "utils.h"
#include "magisk.h"

// Global buffer
char magiskbuf[BUF_SIZE];

void stub(const char *fmt, ...) {}

int main(int argc, char const *argv[]) {
	// Start new thread to monitor logs
	monitor_logs();
	return 0;
}