#include <sys/mount.h>
#include <libgen.h>

#include <utils.hpp>
#include <magisk.hpp>
#include <daemon.hpp>
#include <selinux.hpp>
#include <flags.hpp>

#include "core.hpp"

using namespace std;

[[noreturn]] static void usage() {
    fprintf(stderr,
R"EOF(Magisk - Multi-purpose Utility

Usage: magisk [applet [arguments]...]
   or: magisk [options]...

Options:
   -c                        print current binary version
   -v                        print running daemon version
   -V                        print running daemon version code
   --list                    list all available applets
   --remove-modules          remove all modules and reboot
   --install-module ZIP      install a module zip file

Advanced Options (Internal APIs):
   --daemon                  manually start magisk daemon
   --[init trigger]          start service for init trigger
                             Supported init triggers:
                             post-fs-data, service, boot-complete
   --unlock-blocks           set BLKROSET flag to OFF for all block devices
   --restorecon              restore selinux context on Magisk files
   --clone-attr SRC DEST     clone permission, owner, and selinux context
   --clone SRC DEST          clone SRC to DEST
   --sqlite SQL              exec SQL commands to Magisk database
   --path                    print Magisk tmpfs mount path

Available applets:
)EOF");

    for (int i = 0; applet_names[i]; ++i)
        fprintf(stderr, i ? ", %s" : "    %s", applet_names[i]);
    fprintf(stderr, "\n\n");
    exit(1);
}

int magisk_main(int argc, char *argv[]) {
    if (argc < 2)
        usage();
    if (argv[1] == "-c"sv) {
        printf(MAGISK_VERSION ":MAGISK (" str(MAGISK_VER_CODE) ")\n");
        return 0;
    } else if (argv[1] == "-v"sv) {
        int fd = connect_daemon();
        write_int(fd, CHECK_VERSION);
        string v = read_string(fd);
        printf("%s\n", v.data());
        return 0;
    } else if (argv[1] == "-V"sv) {
        int fd = connect_daemon();
        write_int(fd, CHECK_VERSION_CODE);
        printf("%d\n", read_int(fd));
        return 0;
    } else if (argv[1] == "--list"sv) {
        for (int i = 0; applet_names[i]; ++i)
            printf("%s\n", applet_names[i]);
        return 0;
    } else if (argv[1] == "--unlock-blocks"sv) {
        unlock_blocks();
        return 0;
    } else if (argv[1] == "--restorecon"sv) {
        restorecon();
        return 0;
    } else if (argc >= 4 && argv[1] == "--clone-attr"sv) {;
        clone_attr(argv[2], argv[3]);
        return 0;
    } else if (argc >= 4 && argv[1] == "--clone"sv) {
        cp_afc(argv[2], argv[3]);
        return 0;
    } else if (argv[1] == "--daemon"sv) {
        int fd = connect_daemon(true);
        write_int(fd, START_DAEMON);
        return 0;
    } else if (argv[1] == "--post-fs-data"sv) {
        int fd = connect_daemon(true);
        write_int(fd, POST_FS_DATA);
        return read_int(fd);
    } else if (argv[1] == "--service"sv) {
        int fd = connect_daemon(true);
        write_int(fd, LATE_START);
        return read_int(fd);
    } else if (argv[1] == "--boot-complete"sv) {
        int fd = connect_daemon(true);
        write_int(fd, BOOT_COMPLETE);
        return read_int(fd);
    } else if (argc >= 3 && argv[1] == "--sqlite"sv) {
        int fd = connect_daemon();
        write_int(fd, SQLITE_CMD);
        write_string(fd, argv[2]);
        string res;
        for (;;) {
            read_string(fd, res);
            if (res.empty())
                return 0;
            printf("%s\n", res.data());
        }
    } else if (argv[1] == "--remove-modules"sv) {
        int fd = connect_daemon();
        write_int(fd, REMOVE_MODULES);
        return read_int(fd);
    } else if (argv[1] == "--path"sv) {
        int fd = connect_daemon();
        write_int(fd, GET_PATH);
        string path = read_string(fd);
        printf("%s\n", path.data());
        return 0;
    } else if (argc >= 3 && argv[1] == "--install-module"sv) {
        install_module(argv[2]);
    }
#if 0
    /* Entry point for testing stuffs */
    else if (argv[1] == "--test"sv) {
        return 0;
    }
#endif
    usage();
}
