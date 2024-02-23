#define SYS_INLINE

#include "linux_syscall_support.h"

#define EXPORT_SYMBOL(name) \
__asm__(".global " #name " \n " #name " = sys_" #name)

EXPORT_SYMBOL(_exit);
EXPORT_SYMBOL(open);
EXPORT_SYMBOL(close);
EXPORT_SYMBOL(read);
EXPORT_SYMBOL(write);
EXPORT_SYMBOL(writev);
EXPORT_SYMBOL(unlink);
EXPORT_SYMBOL(mmap);
EXPORT_SYMBOL(readlink);
EXPORT_SYMBOL(unlinkat);
EXPORT_SYMBOL(getpid);
EXPORT_SYMBOL(chdir);
EXPORT_SYMBOL(umount2);
