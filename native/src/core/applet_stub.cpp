#include <sys/stat.h>

#include <magisk.hpp>
#include <selinux.hpp>
#include <base.hpp>

int main(int argc, char *argv[]) {
    if (argc < 1)
        return 1;
    enable_selinux();
    cmdline_logging();
    init_argv0(argc, argv);
    umask(0);
    return APPLET_STUB_MAIN(argc, argv);
}
