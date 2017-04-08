/* daemon.h - Utility functions for daemon-client communication
 */


// Commands require connecting to daemon
typedef enum {
	LAUNCH_MAGISKHIDE,
	STOP_MAGISKHIDE,
	ADD_HIDELIST,
	RM_HIDELIST,
	SUPERUSER,
	TEST
} client_request;

// daemon.c

void start_daemon();
int connect_daemon();

// socket_trans.c

int recv_fd(int sockfd);
void send_fd(int sockfd, int fd);
int read_int(int fd);
void write_int(int fd, int val);
char* read_string(int fd);
void write_string(int fd, const char* val);
