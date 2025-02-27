#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "init.hpp"

__attribute__((constructor))
static void preload_init() {
    // Make sure our next exec won't get bugged
    unsetenv("LD_PRELOAD");
    unlink(PRELOAD_LIB);
}

int security_load_policy(void *data, size_t len) {
    int policy = open(PRELOAD_POLICY, O_WRONLY | O_CREAT, 0644);
    if (policy < 0) return -1;

    // Write the policy
    write(policy, data, len);
    close(policy);

    // Wait for ack
    int ack = open(PRELOAD_ACK, O_RDONLY);
    char c;
    read(ack, &c, 1);
    close(ack);

    return 0;
}
