#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>

int security_load_policy(void *data, size_t len) {
    // Make sure our next exec won't get bugged
    unsetenv("LD_PRELOAD");
    unlink("/dev/preload.so");

    int (*load_policy)(void *, size_t) = dlsym(RTLD_NEXT, "security_load_policy");
    // Skip checking errors, because if we cannot find the symbol, there
    // isn't much we can do other than crashing anyways.
    int result = load_policy(data, len);

    // Wait for ack
    int fd = open("/sys/fs/selinux/enforce", O_RDONLY);
    char c;
    read(fd, &c, 1);
    close(fd);

    return result;
}
