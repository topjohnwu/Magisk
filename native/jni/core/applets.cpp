#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <magisk.hpp>
#include <selinux.hpp>
#include <utils.hpp>

using namespace std;

using main_fun = int (*)(int, char *[]);

static main_fun applet_main[] = { su_client_main, resetprop_main, magiskhide_main, nullptr };

static int call_applet(int argc, char *argv[]) {
    // Applets
    string_view base = basename(argv[0]);
    for (int i = 0; applet_names[i]; ++i) {
        if (base == applet_names[i]) {
            return (*applet_main[i])(argc, argv);
        }
    }
#if ENABLE_INJECT
    if (str_starts(base, "app_process")) {
        return app_process_main(argc, argv);
    }
#endif
    fprintf(stderr, "%s: applet not found\n", base.data());
    return 1;
}

int main(int argc, char *argv[]) {
    umask(0);
    enable_selinux();
    cmdline_logging();
    init_argv0(argc, argv);

    string_view base = basename(argv[0]);
    if (base == "magisk" || base == "magisk32" || base == "magisk64") {
        if (argc > 1 && argv[1][0] != '-') {
            // Calling applet via magisk [applet] args
            --argc;
            ++argv;
        } else {
            return magisk_main(argc, argv);
        }
    }

    return call_applet(argc, argv);
}

